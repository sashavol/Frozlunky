#pragma once

#include "derandom.h"

#define MAX_PLAYERS 4

#define LEVEL_1_1 1
#define LEVEL_5_4 20

#define STATE_PLAYING 0
#define STATE_INPUTLOCK_GENERIC 1
#define STATE_INPUTLOCK_LEVELSTART 2
#define STATE_INPUTLOCK_CIRCLECLOSE 3
#define STATE_GAMEOVER_HUD 30
#define STATE_MAINMENU 15
#define STATE_LEVEL_TRANSITION 11
#define STATE_INTRO 14
#define STATE_TITLE 5
#define STATE_CHARSELECT 17
#define STATE_PLAYERSTATS 21
#define STATE_LOBBY 22
#define STATE_PAUSED 4
#define STATE_WALLS_ARE_SHIFTING 16
//OPT research other states

#define FS_EFFECT_ERR -1
#define FS_EFFECT_OFF  0
#define FS_EFFECT_LOW  1
#define FS_EFFECT_HIGH 2

#define PROHUD_ON  1
#define PROHUD_OFF 0

struct TimeInfo {
	unsigned minutes;
	unsigned seconds;
	double milliseconds;

	TimeInfo() : minutes(0), seconds(0), milliseconds(0.0) {}

	double total_ms() {
		return milliseconds + seconds*1000 + minutes*60*1000;
	}
};

class GameHooks {
public:
	std::shared_ptr<Spelunky> spel;
	std::shared_ptr<DerandomizePatch> dp;

private:
	bool is_valid;

	bool have_steamid;
	std::string steamid;

	Address game_state_offset;
	Address g_CurrentGamePtr;
	Address game_goldcount_offset;
	Address game_timer_offset;
	Address level_timer_offset;
	
	Address player_bomb_offs;
	Address player_health_offs;
	Address player_rope_offs;
	unsigned player_struct_size;
	unsigned player_entity_offs;

	unsigned controller_size;
	unsigned run_switch_offset;
	unsigned ctrl_config_offs;

	unsigned ext_object_offset;
	unsigned menu_object_offset;
	unsigned menu_select_idx_menu_offset;
	unsigned ext_player_size;
	unsigned ext_character_id_offset;
	unsigned ext_fs_effects_offset;
	unsigned ext_dyn_shadows_offset;
	unsigned ext_prohud_offset;
	unsigned ext_fs_offset;

	signed int lvl_hmansion_offset;
	signed int lvl_worm_offset;
	signed int lvl_cog_offset;
	signed int lvl_yeti_offset;
	signed int lvl_blackmkt_offset;
	signed int lvl_mothership_offset;
	signed int lvl_dark_offset;

	BYTE game_damsel_offset;

	signed int ent_grid_offset;

	bool discover_steamid();
	bool discover_gold_count();
	bool discover_timers();
	bool discover_game_state();
	bool discover_player_data();
	bool discover_run_switch_offs();
	bool discover_ctrl_size();
	bool discover_menu_data();
	bool discover_gfx_options();
	bool discover_level_flags();
	bool discover_entity_data();

public:
	GameHooks(std::shared_ptr<Spelunky> spel, std::shared_ptr<DerandomizePatch> dp);
	int game_state();
	int current_level();

	//first player_id = 0
	int bombs(int player_id);
	void set_bombs(int p, int bombs);

	int health(int player_id);
	void set_health(int p, int health);

	int ropes(int player_id);
	void set_ropes(int p, int ropes);

	float player_x(int pid);
	float player_y(int pid);

	int damsel_type();
	void damsel_type(int id);

	signed entity_x_offset();
	signed entity_y_offset();

	Address player_entity(int player_id);
	unsigned player_entity_offset();
	
	unsigned ctrl_size();
	unsigned run_switch_offs();
	Address ctrl_object();
	unsigned ctrl_offset();
	Address ctrl_config(int pid);
	void ctrl_reset_x360(int pid);

	Address game_state_offs();

	unsigned gold_count();
	TimeInfo game_timer();
	TimeInfo level_timer();
	bool valid();
	std::string steam_id();

	Address ext_object();
	int main_menu_select_idx();
	int character_id(int pid);
	void character_id(int pid, int id);

	int fullscreen_effects();
	void set_fullscreen_effects(int state);

	bool dyn_shadows();
	void set_dyn_shadows(bool v);

	bool pro_hud();
	void set_pro_hud(bool v);

	bool fullscreen();

	signed worm_offset();
	signed blackmkt_offset();
	signed haunted_mansion_offset();
	signed yeti_offset();
	signed cog_offset();
	signed mothership_offset();

	signed dark_level_offset();
	void set_dark_level(bool v);
	bool dark_level();

	signed char entity_obj_offset();
	signed int entity_grid_offset();
	signed int entity_row_size();
};