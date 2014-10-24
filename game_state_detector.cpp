#include "game_state_detector.h"

GameStateDetector::GameStateDetector(std::shared_ptr<DerandomizePatch> dp) : 
	spel(dp->spel), 
	is_valid(true), 
	current_id(1),
	last_state(-1)
{
	g_CurrentGamePtr = dp->game_ptr();
	if(g_CurrentGamePtr == 0x0) {
		is_valid = false;
		return;
	}

	game_state_offset = spel->get_stored_hook("game_state_offset");
	if(game_state_offset == 0x0) {
		GameHooks(spel, dp);
		game_state_offset = spel->get_stored_hook("game_state_offset");
		if(game_state_offset == 0x0) {
			is_valid = false;
			return;
		}
	}

	last_state = current_state();
}

GameStateDetector::bind_id GameStateDetector::bind(int state, bind_callback callback, bool every) {
	auto state_binds_iter = binds.find(state);
	if(state_binds_iter == binds.end()) {
		binds[state] = binds_type();
		bind_id id = current_id;
		binds[state][id] = bind_info(callback, every);
		current_id++;
		return id;
	}
	else {
		bind_id id = current_id;
		state_binds_iter->second[id] = bind_info(callback, every);
		current_id++;
		return id;
	}
	return 0;
}

void GameStateDetector::real_unbind(bind_id id) {
	for(auto state_bind_iter = binds.begin(); state_bind_iter != binds.end(); state_bind_iter++) {
		binds_type& state_binds = state_bind_iter->second;
		auto inner_iter = state_binds.find(id);
		if(inner_iter != state_binds.end()) {
			state_binds.erase(inner_iter);
		}
	}
}

void GameStateDetector::request_unbind(bind_id id) {
	unbind_req_mu.lock();
	unbind_requests.push_back(id);
	unbind_req_mu.unlock();
}

void GameStateDetector::cycle() {
	unbind_req_mu.lock();
	for(bind_id unbind_req : unbind_requests) {
		real_unbind(unbind_req);
	}
	unbind_requests.clear();
	unbind_req_mu.unlock();

	int current = current_state();
	bool changed = current != last_state;
	auto iter = binds.find(current);
	if(iter != binds.end()) {
		binds_type& map = iter->second;
		for(auto map_iter = map.begin(); map_iter != map.end(); map_iter++) {
			bind_info& info = map_iter->second;
			if(info.every || changed) {
				info.callback(last_state, current, map_iter->first);
			}
		}
	}
	last_state = current;
}

int GameStateDetector::current_state() {
	Address game;
	spel->read_mem(g_CurrentGamePtr, &game, sizeof(Address));

	int state;
	spel->read_mem(game+game_state_offset, &state, sizeof(int));

	return state;
}
