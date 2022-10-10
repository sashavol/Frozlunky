#include "winheaders.h"

#include "patches.h"
#include "game_hooks.h"
#include "spelunky.h"

#include <string>


#ifdef DEBUG_MODE
#define DISCOVERY_FUNC(FUNC) if(!FUNC()) {std::cout << "GameHooks::" << #FUNC << " failed, hooks invalidated." << std::endl; is_valid = false;}
#else
#define DISCOVERY_FUNC(FUNC) if(!FUNC()) {is_valid = false;}
#endif

GameHooks::GameHooks(std::shared_ptr<Spelunky> spel, std::shared_ptr<DerandomizePatch> dp) : 
	is_valid(true),
	spel(spel), 
	dp(dp),
	have_steamid(false)
{
	g_CurrentGamePtr = dp->game_ptr();
	if(g_CurrentGamePtr == 0x0) {
		is_valid = false;
		return;
	}
	
	DISCOVERY_FUNC(discover_game_state);
	DISCOVERY_FUNC(discover_timers);
	DISCOVERY_FUNC(discover_gold_count);
	DISCOVERY_FUNC(discover_steamid);
	DISCOVERY_FUNC(discover_player_data);
	DISCOVERY_FUNC(discover_ctrl_size);
	DISCOVERY_FUNC(discover_run_switch_offs);
	DISCOVERY_FUNC(discover_menu_data);
	DISCOVERY_FUNC(discover_gfx_options);
	DISCOVERY_FUNC(discover_level_flags);
	DISCOVERY_FUNC(discover_entity_data);
}

Address GameHooks::game_state_offs() {
	return this->game_state_offset;
}


//+9
BYTE ctrl_size_find[] = {0x89,0xFF,0xFF,0xFF,0x8D,0xFF,0xFF,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0x33,0xFF,0x8B,0xFF,0x89};
std::string ctrl_size_mask = "x...x..x.....x.x.x";

//+3 
BYTE ctrl_config_find[] = {0x8B,0xCC,0xCC,0xCC,0x03,0xCC,0x89,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0x6B};
std::string ctrl_config_mask = "x...x.x...x..x.....x";

bool GameHooks::discover_ctrl_size() {
	controller_size = spel->get_stored_hook("controller_size");
	if(controller_size == 0x0) {
		Address ctrl_w = spel->find_mem(ctrl_size_find, ctrl_size_mask);
		if(!ctrl_w) {
			DBG_EXPR(std::cout << "[GameHooks] Failed to find controller size." << std::endl);
			is_valid = false;
			return false;
		}
		ctrl_w += 9;
		spel->read_mem(ctrl_w, &controller_size, sizeof(unsigned));
		DBG_EXPR(std::cout << "[GameHooks] Controller struct size = " << controller_size << std::endl);
	}

	ctrl_config_offs = spel->get_stored_hook("ctrl_config_offs");
	if(!ctrl_config_offs) {
		Address ccfg_cont = spel->find_mem(ctrl_config_find, ctrl_config_mask);
		if(!ccfg_cont) {
			DBG_EXPR(std::cout << "[GameHooks] Failed to find ctrl config offs." << std::endl);
			is_valid = false;
			return false;
		}
		ccfg_cont += 3;

		ctrl_config_offs = 0;
		spel->read_mem(ccfg_cont, &ctrl_config_offs, 1);
		DBG_EXPR(std::cout << "[GameHooks] ctrl_config_offs = " << ctrl_config_offs << std::endl);
	}

	return true;
}

unsigned GameHooks::ctrl_size() {
	return controller_size;
}

//+14
BYTE run_switch_offset_find[] = {0x83,0xFF,0xFF,0x75,0xFF,0x8B,0xFF,0xFF,0xFF,0xFF,0xFF,0x8D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x33,0xFF,0x39,0xFF,0x0F};
std::string run_switch_offset_mask = "x..x.x.....x......x.x.x";

bool GameHooks::discover_run_switch_offs() {
	run_switch_offset = spel->get_stored_hook("run_switch_offset");
	if(run_switch_offset == 0x0) {
		run_switch_offset = spel->find_mem(run_switch_offset_find, run_switch_offset_mask);
		if(!run_switch_offset) {
			DBG_EXPR(std::cout << "[GameHooks] Failed to find run switch offset" << std::endl);
			is_valid = false;
			return false;
		}
		run_switch_offset += 14;
		spel->read_mem(run_switch_offset, &run_switch_offset, sizeof(Address)); //deref
		DBG_EXPR(std::cout << "[GameHooks] Run switch offset = " << run_switch_offset << std::endl);
	}
	return true;
}

unsigned GameHooks::run_switch_offs() {
	return run_switch_offset;
}

BYTE game_state_find[] = {0xBB, 0x0F, 0x00, 0x00, 0x00, 0x3B, 0xC3, 0x75, 0xFF, 0x8B, 0x7E, 0xFF, 0xC7, 0x46, 0xFF, 0x1B, 0x00, 0x00, 0x00, 0x89, 0x5E, 0xFF, 0xE8};
std::string game_state_mask = "xxxxxxxx.xx.xx.xxxxxx.x";

bool GameHooks::discover_game_state() {
	game_state_offset = spel->get_stored_hook("game_state_offset");
	if(game_state_offset == 0x0) 
	{
		Address game_state_ptr = spel->find_mem(game_state_find, game_state_mask);
		if(game_state_ptr == 0x0) {
			return false;
		}
		game_state_ptr += 0x15;

		BYTE byte_state_offset;
		spel->read_mem(game_state_ptr, &byte_state_offset, sizeof(BYTE));

		game_state_offset = (Address)byte_state_offset;
		spel->store_hook("game_state_offset", game_state_offset);

#ifdef DEBUG_MODE
		std::cout << "game_state_ptr -> " << std::setbase(16) << game_state_ptr << std::endl;
		std::cout << "game_state_offset -> " << std::setbase(16) << (Address)game_state_offset << std::endl;
#endif
	}

	return true;
}

BYTE game_timer_offset_find[] = {0xD8, 0xC2, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xD8, 0xD9, 0xDF, 0xE0, 0x84, 0xFF, 0x75, 0xFF, 0xDD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01};
std::string game_timer_offset_mask = "xxx.....xxxxx.x.x.....x";

bool GameHooks::discover_timers() {
	game_timer_offset = spel->get_stored_hook("game_timer_offset");
	if(game_timer_offset == 0x0) 
	{
		Address game_timer_offset_ptr = spel->find_mem(game_timer_offset_find, game_timer_offset_mask);
		if(game_timer_offset_ptr == 0x0) {
			return false;
		}
		game_timer_offset_ptr += 4;

		spel->read_mem(game_timer_offset_ptr, &game_timer_offset, sizeof(Address));
		game_timer_offset -= 8; //this points to milliseconds when read, we want to start from minutes

#ifdef DEBUG_MODE
		std::cout << "game_timer_offset -> " << std::setbase(16) << game_timer_offset << std::endl;
#endif
		spel->store_hook("game_timer_offset", game_timer_offset);
	}

	if(game_timer_offset != 0x0) {
		level_timer_offset = game_timer_offset + 16;
	}

	return true;
}



BYTE game_goldcount_offset_find[] = {0x8B, 0x97, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x50, 0xFF, 0x8B, 0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0x8B, 0x0D, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x81, 0xFF, 0xFF, 0xFF, 0xFF, 0x8B, 0x83, 0x00, 0x00, 0x00, 0x00};
std::string game_goldcount_offset_mask = "xx....xx.xx....xx....xx....xx....";

bool GameHooks::discover_gold_count() {
	game_goldcount_offset = spel->get_stored_hook("game_goldcount_offset");
	if(game_goldcount_offset == 0x0) 
	{
		Address game_goldcount_offset_ptr = spel->find_mem(game_goldcount_offset_find, game_goldcount_offset_mask);
		if(game_goldcount_offset_ptr == 0x0) {
			return false;
		}
		game_goldcount_offset_ptr += 0x17;

		spel->read_mem(game_goldcount_offset_ptr, &game_goldcount_offset, sizeof(Address));

#ifdef DEBUG_MODE
		std::cout << "gold_count_offset -> " << std::setbase(16) << game_goldcount_offset << std::endl;
#endif
		spel->store_hook("game_goldcount_offset", game_goldcount_offset);
	}

	return true;
}


#define STEAMID_MEM_LENGTH 18
BYTE steamid_find[] = "STEAMID=";
std::string steamid_mask = "xxxxxxxx";

#ifdef DEBUG_MODE
bool debugged_steamid = false;
#endif

bool valid_steamid(char id[STEAMID_MEM_LENGTH]) {
	for(int i = 0; i < STEAMID_MEM_LENGTH-1; i++) {
		char ch = id[i];
		if(ch < '0' || ch > '9') {
			return false;
		}
	}

	id[STEAMID_MEM_LENGTH - 1] = 0x0;
	return true;
}

bool GameHooks::discover_steamid() 
{
	char id[STEAMID_MEM_LENGTH];
	Address start = 0x0;
	while(true) {
		start = spel->find_mem(steamid_find, steamid_mask, start);
		if(start == 0x0) {
			have_steamid = false;
			return true;
		}

		spel->read_mem(start+sizeof(steamid_find)-1, &id, sizeof(id));
		if(valid_steamid(id)) {
			break;
		}

		start++;
	}

	this->steamid = std::string(id);
	have_steamid = true;

#ifdef DEBUG_MODE
	std::cout << "Discovered steamid " << this->steamid << " at " << start << std::endl;
#endif

	return true;
}

BYTE playerdata_find[] = {0x33, 0xCC, 0x8B, 0xCC, 0x69, 0xCC, 0xAA, 0xAA, 0xAA, 0xAA, 0xCC, 0x04,
	0x00, 0x00, 0x00, 0x89, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x89, 0xCC, 0xCC, 0xAA, 0xAA, 0xAA, 0xAA, 0x89};
std::string playerdata_mask = "x.x.x......xxxxx......x......x";

//+2
BYTE player_ent_find[] = {0x8B,0xCC,0xAA,0xAA,0xAA,0xAA,0x85,0xCC,0x74,0xCC,0x80,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x74,0xCC,0xC6,0xCC,0xCC,0xCC,0xCC,0x80};
std::string player_ent_mask = "x.....x.x.x......x.x....x";

#ifdef DEBUG_MODE
bool debugged_player_data = false;
#endif

bool GameHooks::discover_player_data() 
{
	Address playerdata_addr = spel->get_stored_hook("pdat");
	if(playerdata_addr == 0x0) {
		playerdata_addr = spel->find_mem(playerdata_find, playerdata_mask);
		if(playerdata_addr == 0x0) {
			return false;
		}
		spel->store_hook("pdat", playerdata_addr);
	}
	
	spel->read_mem(playerdata_addr+6, &player_struct_size, sizeof(unsigned));
	spel->read_mem(playerdata_addr+32, &player_health_offs, sizeof(Address));
	spel->read_mem(playerdata_addr+39, &player_bomb_offs, sizeof(Address));
	spel->read_mem(playerdata_addr+46, &player_rope_offs, sizeof(Address));


	Address pent_container = spel->get_stored_hook("pent_container");
	if(!pent_container) {
		pent_container = spel->find_mem(player_ent_find, player_ent_mask);
		if(!pent_container) {
			return false;
		}
		pent_container += 2;
		spel->store_hook("pent_container", pent_container);
	}

	spel->read_mem(pent_container, &player_entity_offs, sizeof(unsigned));

#ifdef DEBUG_MODE
	if(!debugged_player_data) {
		debugged_player_data = true;
		std::cout << "[GameHooks] Size of a player structure = " << std::setbase(16) << player_struct_size << std::endl;
		std::cout << "[GameHooks] Game health offset = " << std::setbase(16) << player_health_offs << std::endl;
		std::cout << "[GameHooks] Game bomb offset = " << std::setbase(16) << player_bomb_offs << std::endl;
		std::cout << "[GameHooks] Game rope offset = " << std::setbase(16) << player_rope_offs << std::endl;
		std::cout << "[GameHooks] Player entity offset = " << std::setbase(16) << player_entity_offs << std::endl;
	}
#endif

	return true;
}

//+2
BYTE ext_offset_find[] = {0x8B,0xCC,0xCC,0xD9,0xCC,0xD9,0xCC,0xCC,0xCC,0x89,0xCC,0xCC,0xCC,0x81,0xCC,0xCC,0xCC,0xCC,0xCC,0x75,0xCC,0xDD};
std::string ext_offset_mask = "x..x.x...x...x.....x.x";

//+2
BYTE menu_offset_find[] = {0x8B,0xAA,0xAA,0xAA,0xAA,0xAA,0x85,0xAA,0x74,0xAA,0x89,0xAA,0xAA,0xAA,0xAA,0xAA,0xEB,0xAA,0x83};
std::string menu_offset_mask = "x.....x.x.x.....x.x";

//+2
BYTE menu_select_offset_find[] = {0x89,0xCC,0xCC,0xCC,0xCC,0xCC,0xEB,0xCC,0x83,0xCC,0xCC,0x75,0xCC,0x83,0xCC,0xCC,0xCC,0x75,0xCC,0x39};
std::string menu_select_offset_mask = "x.....x.x..x.x...x.x";

//+2
BYTE ext_character_find[] = {0xFF,0xCC,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xD9,0xCC,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0x83};
std::string ext_character_mask = "x.....x.....x.....x.....x";

bool GameHooks::discover_menu_data() {
	//OPT find this dynamically
	ext_player_size = 0x20;
	game_damsel_offset = 0x4;

	ext_object_offset = spel->get_stored_hook("ext_object_offset");
	if(!ext_object_offset) {
		Address container = spel->find_mem(ext_offset_find, ext_offset_mask);
		if(!container) {
			return false;
		}
		container += 2;

		ext_object_offset = 0;
		spel->read_mem(container, &ext_object_offset, 1);
		spel->store_hook("ext_object_offset", ext_object_offset);
	}
	
	menu_object_offset = spel->get_stored_hook("menu_object_offset");
	if(!menu_object_offset) {
		menu_object_offset = spel->find_mem(menu_offset_find, menu_offset_mask);
		if(!menu_object_offset) {
			return false;
		}
		menu_object_offset += 2;

		spel->read_mem(menu_object_offset, &menu_object_offset, sizeof(unsigned));
		spel->store_hook("menu_object_offset", menu_object_offset);
	}
	
	menu_select_idx_menu_offset = spel->get_stored_hook("menu_select_idx_menu_offset");
	if(!menu_select_idx_menu_offset) {
		menu_select_idx_menu_offset = spel->find_mem(menu_select_offset_find, menu_select_offset_mask);;
		if(!menu_select_idx_menu_offset) {
			return false;
		}
		menu_select_idx_menu_offset += 2;

		spel->read_mem(menu_select_idx_menu_offset, &menu_select_idx_menu_offset, sizeof(unsigned));
		spel->store_hook("menu_select_idx_menu_offset", menu_select_idx_menu_offset);
	}

	ext_character_id_offset = spel->get_stored_hook("ext_character_id_offset");
	if(!ext_character_id_offset) {
		ext_character_id_offset = spel->find_mem(ext_character_find, ext_character_mask);
		if(!ext_character_id_offset) {
			return false;
		}
		ext_character_id_offset += 2;

		spel->read_mem(ext_character_id_offset, &ext_character_id_offset, sizeof(unsigned));
		spel->store_hook("ext_character_id_offset", ext_character_id_offset);
	}

	DBG_EXPR(std::cout << "[GameHooks] menu_object_offset = " << menu_object_offset << ", " << "menu_select_idx_menu_offset = " << menu_select_idx_menu_offset << std::endl);
	DBG_EXPR(std::cout << "[GameHooks] ext_object_offset = " << ext_object_offset << std::endl);
	DBG_EXPR(std::cout << "[GameHooks] ext_character_id_offset = " << ext_character_id_offset << std::endl);
	return true;
}

Address GameHooks::ext_object() {
	Address game = dp->game_ptr();
	spel->read_mem(game, &game, sizeof(Address));

	Address ext = 0;
	spel->read_mem(game + ext_object_offset, &ext, sizeof(Address));

	return ext;
}

int GameHooks::main_menu_select_idx() {
	Address ext = ext_object();

	Address menu_obj;
	spel->read_mem(ext + menu_object_offset, &menu_obj, sizeof(Address));

	int menu_idx = -1;
	spel->read_mem(menu_obj + menu_select_idx_menu_offset, &menu_idx, sizeof(int));

	return menu_idx;
}

int GameHooks::character_id(int pid) {
	int id = -1;
	Address ext = ext_object();
	spel->read_mem(ext + ext_character_id_offset + pid * ext_player_size, &id, sizeof(int));
	return id;
}

void GameHooks::character_id(int pid, int id) {
	Address ext = ext_object();
	spel->write_mem(ext + ext_character_id_offset + pid * ext_player_size, &id, sizeof(int));
}

void GameHooks::set_bombs(int p, int bombs) {
	Address game;
	spel->read_mem(dp->game_ptr(), &game, sizeof(Address));
	spel->write_mem(game + player_bomb_offs + player_struct_size*p, &bombs, sizeof(int));
}

int GameHooks::bombs(int player_id) {
	Address game;
	spel->read_mem(dp->game_ptr(), &game, sizeof(Address));

	int bomb_count = 0;
	spel->read_mem(game + player_bomb_offs + player_struct_size*player_id, &bomb_count, sizeof(int));

	return bomb_count;
}

void GameHooks::set_ropes(int p, int ropes) {
	Address game;
	spel->read_mem(dp->game_ptr(), &game, sizeof(Address));
	spel->write_mem(game + player_rope_offs + player_struct_size*p, &ropes, sizeof(int));
}

int GameHooks::ropes(int player_id) {
	Address game;
	spel->read_mem(dp->game_ptr(), &game, sizeof(Address));

	int ropes_count = 0;
	spel->read_mem(game + player_rope_offs + player_struct_size*player_id, &ropes_count, sizeof(int));

	return ropes_count;
}

void GameHooks::set_health(int p, int health) {
	Address game;
	spel->read_mem(dp->game_ptr(), &game, sizeof(Address));
	spel->write_mem(game + player_health_offs + player_struct_size*p, &health, sizeof(int));
}

int GameHooks::health(int player_id) {
	Address game;
	spel->read_mem(dp->game_ptr(), &game, sizeof(Address));

	int health_count = 0;
	spel->read_mem(game + player_health_offs + player_struct_size*player_id, &health_count, sizeof(int));

	return health_count;
}

int GameHooks::damsel_type() {
	Address game = 0;
	spel->read_mem(dp->game_ptr(), &game, sizeof(Address));

	int type = -1;
	spel->read_mem(game + game_damsel_offset, &type, sizeof(int));

	return type;
}

void GameHooks::damsel_type(int d) {
	Address game = 0;
	spel->read_mem(dp->game_ptr(), &game, sizeof(Address));

	spel->write_mem(game + game_damsel_offset, &d, sizeof(int));
}

//OPT replace these with dynamically determined values, they may eventually become invalid if there is a serious update.
#define ENTITY_X_OFFSET 0x30
#define ENTITY_Y_OFFSET 0x34

signed GameHooks::entity_x_offset() {
	return ENTITY_X_OFFSET;
}

signed GameHooks::entity_y_offset() {
	return ENTITY_Y_OFFSET;
}

unsigned GameHooks::player_entity_offset() {
	return player_entity_offs;
}

float GameHooks::player_x(int pid) {
	Address game;
	spel->read_mem(dp->game_ptr(), &game, sizeof(Address));

	Address entity;
	spel->read_mem(game + player_entity_offs + 4*pid, &entity, sizeof(Address));

	float x;
	spel->read_mem(entity + ENTITY_X_OFFSET, &x, sizeof(float));

	return x;
}

float GameHooks::player_y(int pid) {
	Address game;
	spel->read_mem(dp->game_ptr(), &game, sizeof(Address));

	Address entity;
	spel->read_mem(game + player_entity_offs + 4*pid, &entity, sizeof(Address));

	float y;
	spel->read_mem(entity + ENTITY_Y_OFFSET, &y, sizeof(float));

	return y;
}

int GameHooks::game_state() {
	Address game;
	spel->read_mem(g_CurrentGamePtr, &game, sizeof(Address));

	int state;
	spel->read_mem(game+game_state_offset, &state, sizeof(unsigned));

	return state;
}

unsigned GameHooks::gold_count() {
	Address game;
	spel->read_mem(g_CurrentGamePtr, &game, sizeof(Address));

	unsigned gold_count;
	spel->read_mem(game+game_goldcount_offset, &gold_count, sizeof(unsigned));

	return gold_count;
}

TimeInfo GameHooks::game_timer() {
	Address game;
	spel->read_mem(g_CurrentGamePtr, &game, sizeof(Address));

	TimeInfo ti;
	spel->read_mem(game+game_timer_offset, &ti, sizeof(TimeInfo));
	
	return ti;
}

TimeInfo GameHooks::level_timer() {
	Address game;
	spel->read_mem(g_CurrentGamePtr, &game, sizeof(Address));

	TimeInfo ti;
	spel->read_mem(game+level_timer_offset, &ti, sizeof(TimeInfo));
	return ti;
}

int GameHooks::current_level() {
	return dp->current_level();
}

bool GameHooks::valid() {
	return is_valid;
}

std::string GameHooks::steam_id() {
	if(have_steamid) {
		//prevent steam id from being overwritten
		discover_steamid();
		return steamid;
	}
	else
		return "";
}

unsigned GameHooks::ctrl_offset() {
	return 0x40;
}

Address GameHooks::ctrl_object() {
	Address game;
	spel->read_mem(dp->game_ptr(), &game, sizeof(Address));

	Address ctrl;
	spel->read_mem(game + ctrl_offset(), &ctrl, sizeof(Address));

	return ctrl;
}

void GameHooks::ctrl_reset_x360(int pid) {
	Address ctrl = ctrl_object();
	ctrl += pid * controller_size;

	//DEBUG
	std::cout << " writing to " << ctrl << std::endl;

	//OPT make this dynamic
	unsigned val;
	val = 2; spel->write_mem(ctrl + 0x24, &val, sizeof(unsigned));
	val = 0; spel->write_mem(ctrl + 0x28, &val, sizeof(unsigned));
	val = 1; spel->write_mem(ctrl + 0x2C, &val, sizeof(unsigned));
	val = 3; spel->write_mem(ctrl + 0x30, &val, sizeof(unsigned));
	val = 7; spel->write_mem(ctrl + 0x34, &val, sizeof(unsigned));
	val = 5; spel->write_mem(ctrl + 0x38, &val, sizeof(unsigned));
}

Address GameHooks::ctrl_config(int pid) {
	Address ct = ctrl_object();
	return ct + pid*4 + ctrl_config_offs;
}



///////////////////
// GFX OPTIONS
///////////////////

//+2
BYTE fs_effects_find[] = {0xC7,0xCC,0xCC,0xCC,0xCC,0xCC,0x02,0x00,0x00,0x00,0x88,0xCC,0xCC,0xCC,0xE9};
std::string fs_effects_mask = "x.....xxxxx...x";

//+2
BYTE dyn_shadows_find[] = {0x80,0xCC,0xCC,0xCC,0xCC,0xCC,0x00,0x0F,0xCC,0xCC,0xCC,0xCC,0xCC,0xB9,0xCC,0xCC,0xCC,0xCC,0xE8};
std::string dyn_shadows_mask = "x.....xx.....x....x";

//+2
BYTE prohud_find[] = {0x88,0xCC,0xCC,0xCC,0xCC,0xCC,0xE9,0xCC,0xCC,0xCC,0xCC,0x83,0xCC,0xCC,0x75,0xCC,0x83,0xCC,0xCC,0xCC,0xCC,0xCC,0xFF};
std::string prohud_mask = "x.....x....x..x.x.....x";

//+2
BYTE fs_find[] = {
	0x83, 0xCC,  0xCC,  0xCC,  0xCC,  0xCC,  0xCC, 
	0x74, 0xCC, 
	0x3b, 0xCC, 
	0x75, 0xCC, 
	0x83,
};
std::string fs_mask = "x......x.x.x.x";

bool GameHooks::discover_gfx_options() {
	ext_fs_effects_offset = spel->get_stored_hook("fs_effects_offset");
	if(!ext_fs_effects_offset) {
		Address cont = spel->find_mem(fs_effects_find, fs_effects_mask);
		if(!cont) {
			DBG_EXPR(std::cout << "[GameHooks] Failed to find ext_fs_effects_offset" << std::endl);
			return false;
		}
		cont += 2;
		spel->read_mem(cont, &ext_fs_effects_offset, sizeof(unsigned));
		spel->store_hook("fs_effects_offset", ext_fs_effects_offset);
		DBG_EXPR(std::cout << "[GameHooks] ext_fs_effects_offset = " << ext_fs_effects_offset << std::endl);
	}

	ext_dyn_shadows_offset = spel->get_stored_hook("dyn_shadows_offset");
	if(!ext_dyn_shadows_offset) {
		Address cont = spel->find_mem(dyn_shadows_find, dyn_shadows_mask);
		if(!cont) {
			DBG_EXPR(std::cout << "[GameHooks] Failed to find ext_dyn_shadows_offset" << std::endl);
			return false;
		}
		cont += 2;

		spel->read_mem(cont, &ext_dyn_shadows_offset, sizeof(unsigned));
		spel->store_hook("dyn_shadows_offset", ext_dyn_shadows_offset);
		DBG_EXPR(std::cout << "[GameHooks] ext_dyn_shadows_offset = " << ext_dyn_shadows_offset << std::endl);
	}

	ext_prohud_offset = spel->get_stored_hook("prohud_offset");
	if(!ext_prohud_offset) {
		Address cont = spel->find_mem(prohud_find, prohud_mask);
		if(!cont) {
			DBG_EXPR(std::cout << "[GameHooks] Failed to find ext_prohud_offset" << std::endl);
			return false;
		}
		cont += 2;

		spel->read_mem(cont, &ext_prohud_offset, sizeof(unsigned));
		spel->store_hook("prohud_offset", ext_prohud_offset);
		DBG_EXPR(std::cout << "[GameHooks] ext_prohud_offset = " << ext_prohud_offset << std::endl);
	}

	ext_fs_offset = spel->get_stored_hook("ext_fs_offset");
	if(!ext_fs_offset) {
		Address cont = spel->find_mem(fs_find, fs_mask);
		if(!cont) {
			DBG_EXPR(std::cout << "[GameHooks] Failed to find ext_fs_offset" << std::endl);
			return false;
		}
		cont += 2;

		spel->read_mem(cont, &ext_fs_offset, sizeof(unsigned));
		spel->store_hook("ext_fs_offset", ext_fs_offset);
		DBG_EXPR(std::cout << "[GameHooks] ext_fs_offset = " << ext_fs_offset << std::endl);
	}

	return true;
}

int GameHooks::fullscreen_effects() {
	Address ext = ext_object();

	int fs = -1;
	spel->read_mem(ext + ext_fs_effects_offset, &fs, sizeof(int));
	
	return fs;
}

void GameHooks::set_fullscreen_effects(int fs) {
	Address ext = ext_object();
	spel->write_mem(ext + ext_fs_effects_offset, &fs, sizeof(int));
}

bool GameHooks::dyn_shadows() {
	BYTE wut = 0;
	Address ext = ext_object();
	spel->read_mem(ext + ext_dyn_shadows_offset, &wut, sizeof(BYTE));
	return !!wut;
}

void GameHooks::set_dyn_shadows(bool v) {
	BYTE wut = v;
	Address ext = ext_object();
	spel->write_mem(ext + ext_dyn_shadows_offset, &wut, sizeof(BYTE));
}

bool GameHooks::pro_hud() {
	BYTE wut = 0;
	Address ext = ext_object();
	spel->read_mem(ext + ext_prohud_offset, &wut, sizeof(BYTE));
	return !!wut;
}

void GameHooks::set_pro_hud(bool v) {
	BYTE wut = v;
	Address ext = ext_object();
	spel->write_mem(ext + ext_prohud_offset, &wut, sizeof(BYTE));
}

bool GameHooks::fullscreen() {
	BYTE wut = 0;
	Address ext = ext_object();
	spel->read_mem(ext + ext_fs_offset, &wut, sizeof(int));
	return !!wut;
}


//////////
// level flags
//////////

//+7
static BYTE lvl_worm_find[] = {0xE9,0xCC,0xCC,0xCC,0xCC,0x80,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x74,0xCC,0x8D,0xCC,0xCC,0xCC,0x8B,0xCC,0x8B,0xCC,0xE8};
static std::string lvl_worm_mask = "x....x......x.x...x.x.x";

//+4
static BYTE lvl_blackmkt_find[] = {0x7F,0xCC,0x80,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x74,0xCC,0x8D,0xCC,0xCC,0xCC,0x8B,0xCC,0x8B,0xCC,0xE8};
static std::string lvl_blackmkt_mask = "x.x......x.x...x.x.x";

//+4
static BYTE lvl_hmansion_find[] = {0x7F,0xCC,0x80,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x74,0xCC,0x8B,0xCC,0x8D,0xCC,0xCC,0xCC,0x8B,0xCC,0x8B,0xCC,0xE8};
static std::string lvl_hmansion_mask = "x.x......x.x.x...x.x.x";

//+4
static BYTE lvl_yeti_find[] = {0x74,0xCC,0x80,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x74,0xCC,0x8D,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0x8B,0xCC,0x8B};
static std::string lvl_yeti_mask = "x.x......x.x....x.x.x";

//+2
static BYTE lvl_cog_find[] = {0x80,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x74,0xCC,0x33,0xCC,0x83,0xCC,0xCC,0x0F};
static std::string lvl_cog_mask = "x......x.x.x..x";

//+2
static BYTE lvl_mothership_find[] = {0xE9,0xCC,0xCC,0xCC,0xCC,0x80,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x74,0xCC,0x8D,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0x8B,0xCC,0x8B,0xCC,0xE8};
static std::string lvl_mothership_mask = "x....x......x.x....x.x.x.x";

//+2
static BYTE lvl_dark_find[] = {0x80,0xCC,0xCC,0xCC,0xCC,0xCC,0x00,0x74,0xCC,0x8B,0xCC,0xE8,0xCC,0xCC,0xCC,0xCC,0x84,0xCC,0x74,0xCC,0x6A};
static std::string lvl_dark_mask = "x.....xx.x.x....x.x.x";

bool GameHooks::discover_level_flags() {
	lvl_hmansion_offset = 0;
	lvl_worm_offset = 0;
	lvl_cog_offset = 0;
	lvl_yeti_offset = 0;
	lvl_blackmkt_offset = 0;
	lvl_mothership_offset = 0;
	lvl_dark_offset = 0;

	lvl_dark_offset = (signed int)spel->get_stored_hook("lvl_dark_offset");
	if(!lvl_dark_offset) {
		Address cont = spel->find_mem(lvl_dark_find, lvl_dark_mask);
		if(!cont) {
			DBG_EXPR(std::cout << "[GameHooks] Failed to find dark level flag." << std::endl);
			return false;
		}
		spel->read_mem(cont + 2, &lvl_dark_offset, sizeof(signed int));
		spel->store_hook("lvl_dark_offset", lvl_dark_offset);
		DBG_EXPR(std::cout << "[GameHooks] lvl_dark_offset = " << lvl_dark_offset << std::endl);
	}

	lvl_worm_offset = spel->get_stored_hook("lvl_worm_offset");
	if(!lvl_worm_offset) {
		Address worm_cont = spel->find_mem(lvl_worm_find, lvl_worm_mask);
		if(!worm_cont) {
			DBG_EXPR(std::cout << "[GameHooks] Failed to find worm level flag." << std::endl);
			return false;
		}
		spel->read_mem(worm_cont + 7, &lvl_worm_offset, sizeof(signed int));
		spel->store_hook("lvl_worm_offset", lvl_worm_offset);
		DBG_EXPR(std::cout << "[GameHooks] lvl_worm_offset = " << lvl_worm_offset << std::endl);
	}

	lvl_blackmkt_offset = spel->get_stored_hook("lvl_blackmkt_offset");
	if(!lvl_blackmkt_offset) {
		Address blackmkt_cont = spel->find_mem(lvl_blackmkt_find, lvl_blackmkt_mask);
		if(!blackmkt_cont) {
			DBG_EXPR(std::cout << "[GameHooks] Failed to find black market flag." << std::endl);
			return false;
		}
		spel->read_mem(blackmkt_cont + 4, &lvl_blackmkt_offset, sizeof(signed int));
		spel->store_hook("lvl_blackmkt_offset", lvl_blackmkt_offset);
		DBG_EXPR(std::cout << "[GameHooks] lvl_blackmkt_offset = " << lvl_blackmkt_offset << std::endl);
	}

	lvl_hmansion_offset = spel->get_stored_hook("lvl_hmansion_offset");
	if(!lvl_hmansion_offset) {
		Address hmansion_cont = spel->find_mem(lvl_hmansion_find, lvl_hmansion_mask);
		if(!hmansion_cont) {
			DBG_EXPR(std::cout << "[GameHooks] Failed to find haunted mansion flag." << std::endl);
			return false;
		}
		spel->read_mem(hmansion_cont + 4, &lvl_hmansion_offset, sizeof(signed int));
		spel->store_hook("lvl_hmansion_offset", lvl_hmansion_offset);
		DBG_EXPR(std::cout << "[GameHooks] lvl_hmansion_offset = " << lvl_hmansion_offset << std::endl);
	}

	lvl_yeti_offset = spel->get_stored_hook("lvl_yeti_offset");
	if(!lvl_yeti_offset) {
		Address yeti_cont = spel->find_mem(lvl_yeti_find, lvl_yeti_mask);
		if(!yeti_cont) {
			DBG_EXPR(std::cout << "[GameHooks] Failed to find yeti level flag." << std::endl);
			return false;
		}
		spel->read_mem(yeti_cont + 4, &lvl_yeti_offset, sizeof(signed int));
		spel->store_hook("lvl_yeti_offset", lvl_yeti_offset);
		DBG_EXPR(std::cout << "[GameHooks] lvl_yeti_offset = " << lvl_yeti_offset << std::endl);
	}

	lvl_cog_offset = spel->get_stored_hook("lvl_cog_offset");
	if(!lvl_cog_offset) {
		Address cog_cont = spel->find_mem(lvl_cog_find, lvl_cog_mask);
		if(!cog_cont) {
			DBG_EXPR(std::cout << "[GameHooks] Failed to find city of gold level flag." << std::endl);
			return false;
		}
		spel->read_mem(cog_cont + 2, &lvl_cog_offset, sizeof(signed int));
		spel->store_hook("lvl_cog_offset", lvl_cog_offset);
		DBG_EXPR(std::cout << "[GameHooks] lvl_cog_offset = " << lvl_cog_offset << std::endl);
	}

	lvl_mothership_offset = spel->get_stored_hook("lvl_mothership_offset");
	if(!lvl_mothership_offset) {
		Address ms_cont = spel->find_mem(lvl_mothership_find, lvl_mothership_mask);
		if(!ms_cont) {
			DBG_EXPR(std::cout << "[GameHooks] Failed to find mothership flag." << std::endl);
			return false;
		}
		spel->read_mem(ms_cont + 7, &lvl_mothership_offset, sizeof(signed int));
		spel->store_hook("lvl_mothership_offset", lvl_mothership_offset);
		DBG_EXPR(std::cout << "[GameHooks] lvl_mothership_offset = " << lvl_mothership_offset << std::endl);
	}

	return true;
}

signed GameHooks::worm_offset() {
	return lvl_worm_offset;
}

signed GameHooks::blackmkt_offset() {
	return lvl_blackmkt_offset;
}

signed GameHooks::haunted_mansion_offset() {
	return lvl_hmansion_offset;
}

signed GameHooks::yeti_offset() {
	return lvl_yeti_offset;
}

signed GameHooks::cog_offset() {
	return lvl_cog_offset;
}

signed GameHooks::mothership_offset() {
	return lvl_mothership_offset;
}

signed GameHooks::dark_level_offset() {
	return lvl_dark_offset;
}

void GameHooks::set_dark_level(bool v) {
	Address game;
	spel->read_mem(dp->game_ptr(), &game, sizeof(Address));

	char val = v ? 1 : 0;
	spel->write_mem(game + dark_level_offset(), &val, sizeof(char));
}

bool GameHooks::dark_level() {
	Address game;
	spel->read_mem(dp->game_ptr(), &game, sizeof(Address));

	char val;
	spel->read_mem(game + dark_level_offset(), &val, sizeof(char));

	return !!val;
}


//////////////////
// ENTITY DATA  //
//////////////////

//WARN static definitions
#define ENTITY_OBJ_OFFSET 0x3C
#define ENTITY_GRID_ROW_SIZE 46

//+3
static BYTE entity_grid_find[] = {0x83,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x00,0x8D,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x89,0xCC,0xCC,0xCC,0x0F};
static std::string entity_grid_mask = "x......xx......x...x";

bool GameHooks::discover_entity_data() {
	ent_grid_offset = spel->get_stored_hook("ent_grid_offset");
	if(!ent_grid_offset) {
		Address cont = spel->find_mem(entity_grid_find, entity_grid_mask);
		if(!cont) {
			DBG_EXPR(std::cout << "[GameHooks] Failed to find ent_grid_offset" << std::endl);
			return false;
		}
		
		spel->read_mem(cont+3, &ent_grid_offset, sizeof(signed int));
		spel->store_hook("ent_grid_offset", ent_grid_offset);

		DBG_EXPR(std::cout << "[GameHooks] ent_grid_offset = " << ent_grid_offset << std::endl);
	}

	return true;
}

signed int GameHooks::entity_row_size() {
	return ENTITY_GRID_ROW_SIZE;
}

signed int GameHooks::entity_grid_offset() {
	return ent_grid_offset;
}

signed char GameHooks::entity_obj_offset() {
	return ENTITY_OBJ_OFFSET;
}