#pragma once

#include "patches.h"
#include "game_hooks.h"

#include <functional>
#include <chrono>

#define NG_STATE_WAITING 1
#define NG_STATE_WAITING2 2
#define NG_STATE_COMPLETED 4

//a light-weight one time disposable detector for a new spelunky game
class NewGameDetector {
public:
	typedef std::function<void()> newgame_callback;
	typedef std::function<void(bool)> potential_init_callback;

private:
	typedef std::chrono::high_resolution_clock::time_point time_point;
	
	potential_init_callback init_cb;
	newgame_callback newgame_cb;
	std::shared_ptr<GameHooks> hooks;
	int init_state;
	time_point waiting_point;
	bool is_empty;

public:
	NewGameDetector(std::shared_ptr<GameHooks> hooks, newgame_callback ng, potential_init_callback uncertain_init) : 
		init_cb(uncertain_init), 
		newgame_cb(ng), 
		hooks(hooks), 
		init_state(NG_STATE_WAITING) 
	{
		is_empty = !hooks || (!ng && !uncertain_init);
	}

	NewGameDetector(std::shared_ptr<GameHooks> hooks, newgame_callback ng) :
		newgame_cb(ng), 
		hooks(hooks), 
		init_state(NG_STATE_WAITING) 
	{
		is_empty = !hooks || !ng;
	}

	//empty ngd
	NewGameDetector() : is_empty(true) {}

	bool empty() {
		return is_empty;
	}

	void cycle();
};