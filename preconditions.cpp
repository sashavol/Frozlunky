#include "preconditions.h"

NetplayPreconditions::NetplayPreconditions(std::shared_ptr<DerandomizePatch> d, std::shared_ptr<GameHooks> g) 
	: spel(d->spel), dp(d), gh(g)
{}

bool NetplayPreconditions::valid() {
	return is_valid;
}

std::string NetplayPreconditions::FriendlyString(int npp) {
	switch(npp) {
	case NPP_OK:
		return "";
	case NPP_ERROR_INCORRECT_SAVEFILE:
		return "You are currently using a different save file.";
	case NPP_ERROR_MAIN_MENU:
		return "Before starting Netplay, please go to the Spelunky main menu and position your selection on 'Achievements'";
	case NPP_ERROR_FULLSCREEN:
		return "Before starting Netplay, please set Spelunky to windowed mode in graphics options.";
	default:
		return "";
	}
}

int NetplayPreconditions::check() {
	if(gh->fullscreen()) {
		return NPP_ERROR_FULLSCREEN;
	}
	if(!(gh->game_state() == STATE_MAINMENU && gh->main_menu_select_idx() == 3)) { //main menu index 3 = Achievement
		return NPP_ERROR_MAIN_MENU;
	}
	else {
		return NPP_OK;
	}
}

void NetplayPreconditions::load_savefile() {}