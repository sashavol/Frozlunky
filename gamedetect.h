#pragma once

#include "patches.h"
#include "debug.h"
#include "derandom.h"
#include "game_hooks.h"
#include "oneplayer_only.h"

class GameChangeDetector {
private:
	std::shared_ptr<DerandomizePatch> dp;
	std::shared_ptr<Spelunky> spel;
	std::shared_ptr<GameHooks> gh;
	std::shared_ptr<OnePlayerOnlyPatch> opop;
	Address games_game_offset;
	int last_games;
	int last_deaths;
	bool is_valid;
	bool last_is_gameover;

	int game_count();
	bool attempt_game_offset(BYTE* game_offset_find, const std::string& game_offset_mask, Address offset);

public:
	GameChangeDetector(std::shared_ptr<DerandomizePatch> dp, std::shared_ptr<GameHooks> gh);

	bool game_success();
	bool game_changed();
	bool valid();
};