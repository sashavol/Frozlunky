#include "new_game_detector.h"

void NewGameDetector::cycle() {
	if(!hooks) {
		return;
	}

	if(init_state == NG_STATE_WAITING) {
		waiting_point = std::chrono::high_resolution_clock::now();
		if(hooks->current_level() == 1 && hooks->game_state() == STATE_INPUTLOCK_GENERIC) {
			if(init_cb) {
				init_cb(false);
			}

			init_state = NG_STATE_WAITING2;
		}
	}
	else if(init_state == NG_STATE_WAITING2) {
		if(hooks->game_state() == STATE_PLAYING && hooks->current_level() == 1) {
			if(newgame_cb) {
				newgame_cb();
			}

			init_state = NG_STATE_COMPLETED;
		}
		
		auto now = std::chrono::high_resolution_clock::now();
		
		if(std::chrono::duration_cast<std::chrono::milliseconds>(waiting_point - now).count() >= 1500) {
			if(init_cb) {
				init_cb(true);
			}

			init_state = NG_STATE_WAITING;
		}
	}
}