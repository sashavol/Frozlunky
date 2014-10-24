#pragma once

#include "spelunky.h"
#include "seeder.h"
#include "gamedetect.h"
#include "remove_daily.h"
#include "patch_group.h"
#include "new_game_detector.h"
#include "oneplayer_only.h"
#include "game_hooks.h"
#include "custom_hud.h"

#include "frozboards/score_info.h"
#include "frozboards/session.h"

#include <chrono>
#include <map>
#include <vector>
#include <functional>

#define DAILY_INVALID 0
#define DAILY_WAITING 1
#define DAILY_WAITING2 2
#define DAILY_INPROGRESS 3
#define DAILY_COMPLETED 4

#define UPDATE_INTERVAL_MS 100

#define FROZBOARDS_URL "http://frozboards.sashavol.com"

class DailyInstance {
public:
	typedef std::chrono::high_resolution_clock::time_point time_point;
	typedef std::map<time_point, Frozboards::TrackedValues> points_data;


private:
	time_point waiting_point;
	unsigned current_status;
	std::shared_ptr<Frozboards::Session> session;
	std::shared_ptr<Spelunky> spel;
	std::shared_ptr<Seeder> seeder;
	std::shared_ptr<GameHooks> hooks;
	std::shared_ptr<DerandomizePatch> dp;
	std::shared_ptr<PatchGroup> core;
	std::shared_ptr<PatchGroup> special;
	std::shared_ptr<OnePlayerOnlyPatch> opop;
	std::shared_ptr<CustomHudPatch> chp;

	NewGameDetector ngd;
	GameChangeDetector gcd;
	std::string seed;

	double score;

	bool locked_seed;
	bool one_player_only;

	std::map<time_point, Frozboards::TrackedValues> points;
	Frozboards::TrackedValuesSet important;

private:
	void submit_score(bool success);
	Frozboards::TrackedValues track_values();
	bool validate_points();
	void progress_cycle();
	void waiting_cycle();
	Frozboards::TrackedValues append_lastlevel_score();

public:
	DailyInstance(std::shared_ptr<Frozboards::Session> session,
		std::shared_ptr<Spelunky> spel, 
		std::shared_ptr<Seeder> seeder, 
		std::shared_ptr<DerandomizePatch> dp,
		std::shared_ptr<CustomHudPatch> chp,
		std::shared_ptr<PatchGroup> core_patches,
		std::shared_ptr<PatchGroup> special_patches);

	~DailyInstance();
	void cycle();
	unsigned status();
	double total_score();

	void force_end() {
		this->current_status = DAILY_INVALID;
	}

	std::shared_ptr<Frozboards::Session> get_session() {
		return this->session;
	}

private:
	static void ForceLeaderboardStatus(bool value);
	static void OnLeaderboard(std::function<void(bool)> result_callback);

public:
	static void Available(std::shared_ptr<Spelunky> spel, std::shared_ptr<DerandomizePatch> dp, std::function<void(bool)> result_callback);
};