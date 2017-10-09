#pragma once

#include <functional>

#include "derandom.h"
#include "game_hooks.h"
#include "oneplayer_only.h"

#define NPP_OK							 0
#define NPP_ERROR_MAIN_MENU				(1 << 0)
#define NPP_ERROR_INCORRECT_SAVEFILE	(1 << 1)
#define NPP_ERROR_INCORRECT				(1 << 2)
#define NPP_ERROR_FULLSCREEN			(1 << 3)

class NetplayPreconditions {
private:
	bool is_valid;

	Address menu_selection_addr;
	std::shared_ptr<Spelunky> spel;
	std::shared_ptr<GameHooks> gh;
	std::shared_ptr<DerandomizePatch> dp;

public:
	NetplayPreconditions(std::shared_ptr<DerandomizePatch> dp, std::shared_ptr<GameHooks> gh);
	bool valid();

	//checks that netplay preconditions are met
	int check();
	
	//resolves NPP_ERROR_INCORRECT_SAVEFILE
	void load_savefile();

	static std::string FriendlyString(int npp_code);
};