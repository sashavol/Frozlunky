#include "daily.h"

#include <cmath>
#include <vector>


void DailyInstance::Available(std::shared_ptr<Spelunky> spel, std::shared_ptr<DerandomizePatch> dp, std::function<void(bool)> result_callback) 
{
	if(!GameHooks(spel, dp).valid()) {
		result_callback(false);
		return;
	}
	
	if(!OnePlayerOnlyPatch(spel, dp).valid()) {
		result_callback(false);
		return;
	}

	OnLeaderboard([result_callback](bool result) {
		result_callback(!result);
	});
}


DailyInstance::DailyInstance(std::shared_ptr<Frozboards::Session> session,
							 std::shared_ptr<Spelunky> spel, 
							 std::shared_ptr<Seeder> seeder, 
							 std::shared_ptr<DerandomizePatch> dp, 
							 std::shared_ptr<CustomHudPatch> chp,
							 std::shared_ptr<PatchGroup> core_patches,
							 std::shared_ptr<PatchGroup> special_patches) :
	session(session),
	spel(spel), 
	seeder(seeder), 
	current_status(DAILY_INVALID), 
	hooks(std::make_shared<GameHooks>(spel, dp)), 
	gcd(dp, hooks), 
	dp(dp),
	score(0.0),
	locked_seed(false),
	one_player_only(false),
	core(core_patches),
	special(special_patches),
	chp(chp)
{
	if(session->invalidated()) {
		current_status = DAILY_INVALID;
		return;
	}

	if(!gcd.valid()) {
		current_status = DAILY_INVALID;
		return;
	}

	if(!chp->is_active()) {
		chp->perform();
	}

	seed = session->session_seed();

	if(seed == "~ERR_ONLEAD") {
		current_status = DAILY_INVALID;
		return;
	}

	if(!hooks->valid()) {
#ifdef DEBUG_MODE
		std::cout << "Warning: Daily failed to initialize due to invalid hooks." << std::endl;
#endif

		current_status = DAILY_INVALID;
		return;
	}

	opop = std::make_shared<OnePlayerOnlyPatch>(spel, dp);
	if(!opop->valid()) {
#ifdef DEBUG_MODE
		std::cout << "Warning: Daily failed to initialize due to invalid one player only patch." << std::endl;
#endif
		current_status = DAILY_INVALID;
		return;
	}

	if(opop->controller_count() > 1) {
#ifdef DEBUG_MODE
		std::cout << "Warning: More than 1 controllers enabled, disabling. (Emergency catch)" << std::endl;
#endif
		current_status = DAILY_INVALID;
		return;
	}

	opop->perform();
	one_player_only = true;
	
	if(session->run_count() == 0) {
		chp->set_text(L"Start a game to begin the Daily.");
	}

	current_status = DAILY_WAITING;
}

DailyInstance::~DailyInstance() {
	if(one_player_only) {
		opop->undo();
	}

	special->unlock();
	core->unlock();
	seeder->unlock();

	if(locked_seed) {
		core->pop_state();
		special->pop_state();

		seeder->pop_seed();
	}
}

unsigned DailyInstance::status() {
	return current_status;
}

double DailyInstance::total_score() {
	return score;
}

bool cached = false;
bool cached_value;

void DailyInstance::OnLeaderboard(std::function<void(bool)> result_callback) {
	if(cached) {
		result_callback(cached_value);
	}
	else {
		//TODO
		result_callback(false);
	}
}

void DailyInstance::ForceLeaderboardStatus(bool value) {
	cached_value = value;
	cached = true;
}





void DailyInstance::submit_score(bool success) {
	session->submit(important, success);
}


inline double compute_score(const Frozboards::TrackedValues& tv) {
	return Frozboards::Session::ComputePointScore(tv);
}

Frozboards::TrackedValues DailyInstance::append_lastlevel_score() {
	Frozboards::TrackedValues lastlevel_point;
	double last_level_score = -1.0;
	for(auto point = points.rbegin(); point != points.rend(); point++) {
		lastlevel_point = point->second;
		if(lastlevel_point.level_time > 10) {
			last_level_score = compute_score(lastlevel_point);
			break;
		}
	}

	if(last_level_score < 0) {
		MessageBox(NULL, "An error occurred during the daily: Unable to determine last level score. Please report this issue.", "Frozboards", MB_OK);
		current_status = DAILY_INVALID;
		return lastlevel_point;
	}

	score += last_level_score;
	session->process_score(score);

	return lastlevel_point;
}

Frozboards::TrackedValues DailyInstance::track_values() 
{
	Frozboards::TrackedValues tv;
	bool point_of_interest = false;
	//!TODO track more values.

	TimeInfo ti = hooks->level_timer();
	TimeInfo ti_g = hooks->game_timer();
	tv.bombs = hooks->bombs(0);
	tv.gold = hooks->gold_count();
	tv.health = hooks->health(0);
	tv.level = hooks->current_level();
	tv.level_time = ti.total_ms();
	tv.ropes = hooks->ropes(0);
	tv.total_time = ti_g.total_ms();

	double level_score = compute_score(tv);
	double last_time;
	if(points.size() > 0) {
		last_time = points.rbegin()->second.level_time;
	}
	else {
		last_time = -1;
	}

	int current = hooks->current_level();
	if(last_time > ti.total_ms()) {
		Frozboards::TrackedValues lastlevel_point = append_lastlevel_score();
		point_of_interest = true;
		important.push_back(lastlevel_point);
	}

	if(important.empty() || (tv.total_time - important.rbegin()->total_time) >= 1000) {
		point_of_interest = true;
	}

	double best_score = session->best();
	wchar_t print[256];
	if(tv.level_time < 1400) {
		std::swprintf(print, sizeof(print), L"PB: %.1f  Score: %.1f  Possible: ...  (%d/%d)", best_score, score, session->run_count()+1, SESSION_MAX_ATTEMPTS);
	}
	else {
		std::swprintf(print, sizeof(print), L"PB: %.1f  Score: %.1f  Possible: %.1f  (%d/%d)", best_score, score, score+level_score, session->run_count()+1, SESSION_MAX_ATTEMPTS);
	}
	
	chp->set_text(print);
	

	if(point_of_interest) {
		important.push_back(tv);
	}

	points[std::chrono::high_resolution_clock::now()] = tv;

	return tv;
}

void DailyInstance::progress_cycle() {
	time_point now = std::chrono::high_resolution_clock::now();

	if(points.empty() 
		|| std::chrono::duration_cast<std::chrono::milliseconds>(now - points.rbegin()->first).count() >= UPDATE_INTERVAL_MS) 
	{
		int game_state = hooks->game_state();
		
		if(game_state != STATE_PAUSED) {
			track_values();
		}

		if(gcd.game_changed()) {
			//final point is important
			important.push_back(points.rbegin()->second);

			bool success = hooks->game_state() != STATE_GAMEOVER_HUD && hooks->current_level() > 1;
			if(success) {
				append_lastlevel_score(); //add boss fight to score count
				score += 650.0;
			}


			submit_score(success);
			session->process_score(score);
			current_status = DAILY_COMPLETED;
			return;
		}

		//aborted daily via main menu
		if(game_state == STATE_MAINMENU) {
			current_status = DAILY_INVALID;
			return;
		}
	}
}


void DailyInstance::waiting_cycle() 
{
	//initialize ngd here to prevent flow confusion
	if(ngd.empty()) 
	{
		ngd = NewGameDetector(hooks, 
			//level initialized fully
			[this]() {
				gcd.game_changed(); //warm up the gcd so it doesn't do a false trigger
				current_status = DAILY_INPROGRESS;
			},

			//pre level init operations with undo capability
			[this](bool invalidate) {
				if(!invalidate) {
					//save core state and perform all core patches
					core->unlock();
					core->push_state();
					core->perform();
					core->lock();

					//save all special states and undo them 
					//OPT in the future we may want special patches in daily seeds, so perhaps add more context to seed retrieval?
					special->unlock();
					special->push_state();
					special->undo();
					special->lock();

					//save the existing seed and load the daily seed
					this->seeder->unlock();
					this->seeder->push_seed();
					this->seeder->seed(seed);
					this->seeder->lock();

					locked_seed = true;
					
					if(!chp->is_active()) {
						chp->unlock();
						chp->perform();
					}
					chp->set_text(L"");

					current_status = DAILY_WAITING2;
				}
				else {
					core->unlock();
					special->unlock();
					core->pop_state();
					special->pop_state();
					this->seeder->unlock();
					this->seeder->pop_seed();
					locked_seed = false;

					current_status = DAILY_WAITING;
				}
			}
		);
	}

	ngd.cycle();
}

void DailyInstance::cycle() {
	switch(current_status) {
	case DAILY_WAITING2:
	case DAILY_WAITING:
		waiting_cycle();
		break;
	case DAILY_INPROGRESS:
		progress_cycle();
		break;
	}
}


//TODO
bool DailyInstance::validate_points() {
	return false;
}
