#pragma once

#include "patches.h"
#include "derandom.h"
#include "tile_patch.h"
#include "game_hooks.h"
#include <functional>

namespace TileEditing {
	bool Initialize(std::shared_ptr<Spelunky> spel, std::shared_ptr<DerandomizePatch> dp, std::shared_ptr<GameHooks> gh);
	bool Valid();
	bool Visible();
	void DisplayStateCallback(std::function<void(bool)> cb);
	void ShowUI();
	void HideUI();
	std::shared_ptr<::Patch> GetPatch();
}