#include "gamedetect.h"

BYTE game_offset_find_gog[] = {0x55, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x39, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0x01};
const std::string game_offset_mask_gog = "x.xxxxx.....xx....x";


BYTE game_offset_find_reg[] = {0x0F, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x9F, 0x00, 0x00, 0x00, 0x00, 0x01, 0x9F, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x8B, 0x35};
const std::string game_offset_mask_reg = "x.....x.....x.....x.....xx.";

//TODO GOG support

int GameChangeDetector::game_count() {
	Address game_ptr = dp->game_ptr();
	Address game;
	dp->spel->read_mem(game_ptr, &game, sizeof(Address));

	Address game_addr = game + games_game_offset;
	int count = -1;
	dp->spel->read_mem(game_addr, &count, sizeof(int));
	return count;
}

bool GameChangeDetector::attempt_game_offset(BYTE* game_offset_find, const std::string& game_offset_mask, Address offset) 
{
	Address offset_container = dp->spel->find_mem(game_offset_find, game_offset_mask);

	if(offset_container == 0x0) {
		return false;
	}

	offset_container += offset;
	dp->spel->read_mem(offset_container, &games_game_offset, sizeof(Address));

#ifdef DEBUG_MODE
	std::cout << "Game count container is " << std::setbase(16) << offset_container << std::endl;
	std::cout << "Game count offset is " << std::setbase(16) << games_game_offset << std::endl;
#endif

	dp->spel->store_hook("dgoffs", games_game_offset);

	return true;
}

GameChangeDetector::GameChangeDetector(std::shared_ptr<DerandomizePatch> dp, std::shared_ptr<GameHooks> gh) : 
	dp(dp), 
	gh(gh), 
	spel(dp->spel), 
	last_games(-1), 
	is_valid(true),
	opop(std::make_shared<OnePlayerOnlyPatch>(dp->spel, dp))
{
	games_game_offset = dp->spel->get_stored_hook("dgoffs");
	if(games_game_offset == 0x0) {
		if(!attempt_game_offset(game_offset_find_reg, game_offset_mask_reg, 8)
		&& !attempt_game_offset(game_offset_find_gog, game_offset_mask_gog, 20)) 
		{
#ifdef DEBUG_MODE
			std::cout << "WARNING: Unable to find game count offset." << std::endl;
#endif
			is_valid = false;
		}
	}
}

bool GameChangeDetector::game_changed() {
	int curr = game_count();

	//alternative method using game over hud if > 1 player, only possible if OnePlayerOnlyPatch is valid (since that contains a hook for player count)
	if(opop->valid() && opop->controller_count() > 1) 
	{
		if(!last_is_gameover && (gh->game_state() == STATE_GAMEOVER_HUD)) {
			last_games = curr;
			last_is_gameover = true;
			return true;
		}
		
		if(gh->game_state() != STATE_GAMEOVER_HUD) {
			last_is_gameover = false;
		}
	
		return false;
	}
	else 
	{
		//otherwise try game count
		if(last_games != curr) {
			last_games = curr;
			return true;
		}
		else {
			return false;
		}
	}
}

bool GameChangeDetector::valid() {
	return is_valid;
}