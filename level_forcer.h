#pragma once

#include "patches.h"
#include "derandom.h"
#include "game_hooks.h"
#include <thread>
#include <atomic>
#include <mutex>

#define LF_BLACK_MARKET (1 << 0)
#define LF_CITY_OF_GOLD (1 << 1)
#define LF_MOTHERSHIP   (1 << 2)
#define LF_WORM			(1 << 3)
#define LF_YETI			(1 << 4)
#define LF_HAUNTED_MANSION (1 << 5)

#define LR_BLACK_MARKET (LF_BLACK_MARKET << 5)
#define LR_CITY_OF_GOLD (LF_CITY_OF_GOLD << 5)
#define LR_MOTHERSHIP (LF_MOTHERSHIP << 5)
#define LR_WORM (LF_WORM << 5)
#define LR_YETI (LF_YETI << 5)
#define LR_HAUNTED_MANSION (LF_HAUNTED_MANSION << 5)

#define LB_DEFAULT_START 1
#define LB_DEFAULT_OLMEC 16
#define LB_DEFAULT_YAMA 20

class LevelForcer {
private:
	std::shared_ptr<Spelunky> spel;
	std::shared_ptr<DerandomizePatch> dp;
	std::shared_ptr<GameHooks> gh;

	std::atomic<int> lvl;
	std::atomic<unsigned> flags;
	
	std::atomic<bool> active;

	bool is_valid;

public:
	~LevelForcer();
	LevelForcer(std::shared_ptr<DerandomizePatch> dp, std::shared_ptr<GameHooks> gh);

	void cycle();
	void force(int lvl, unsigned flags=0);

	bool valid();

	bool enabled();
	void set_enabled(bool enabled);
};


class LevelRedirect {
private:
	bool reset_checkpoint;
	int last_level_start;
	int last_level_olmec;
	int last_level_yama;

public:
	std::atomic<int> level_start;
	std::atomic<int> level_olmec;
	std::atomic<int> level_yama;

	std::mutex checkpoint_mutex;
	std::atomic<int> last_checkpoint;
	std::atomic<bool> checkpoint_mode;
	
	std::atomic<bool> game_started;
	std::atomic<int> last_state;
	
private:
	std::shared_ptr<Spelunky> spel;
	std::shared_ptr<GameHooks> gh;

	void write_level(int lvl);
	int current_level();

public:
	LevelRedirect(std::shared_ptr<GameHooks> gh);

	void reset();
	void cycle();
	bool valid();
};