#pragma once

#include "remote_call_patch.h"
#include "rc.h"
#include "spelunky.h"
#include "derandom.h"
#include "game_hooks.h"

#include <functional>

class SaveManager {
private:
	Address whitescreen_addr;
	BYTE* whitescreen_orig;
	
	Address options_addr;
	BYTE* options_orig;

	std::shared_ptr<GameHooks> gh;
	std::shared_ptr<RemoteCallPatch> rcp;
	std::shared_ptr<DerandomizePatch> dp;
	bool is_valid;

	bool write_savefile();
	void patch();
	void unpatch();

public:
	SaveManager(std::shared_ptr<RemoteCallPatch> rcp, std::shared_ptr<GameHooks> gh, std::shared_ptr<DerandomizePatch> dp);
	~SaveManager();

public:
	void load_netplay_save(std::function<void(bool)> cb = std::function<void(bool)>());
	bool valid();
};