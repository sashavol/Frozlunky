#pragma once

#include <functional>
#include <memory>

#include "spelunky.h"
#include "derandom.h"
#include "seeder.h"
#include "patch_group.h"
#include "game_hooks.h"

#define NETPLAY_TITLE "Netplay"

namespace NetplayGUI {
	//calls the callback when netplay session started / ended
	void NetplayChangeCallback(std::function<void(bool)> active_cb);

	//calls the callback when window is displayed / closed
	void VisibilityChangeCallback(std::function<void(bool)> visib_cb);

	//displays the netplay gui (subsequently calling the visibility change callback)
	void DisplayNetplayGUI();

	//hides the netplay gui (subsequently calling the visibility change callback)
	void HideNetplayGUI();

	//initializes the netplay gui with necessary components
	bool Init(std::shared_ptr<Spelunky> spel,
		std::shared_ptr<DerandomizePatch> dp,
		std::shared_ptr<Seeder> seeder,
		std::shared_ptr<GameHooks> gh);

	int PlayerID();
}	