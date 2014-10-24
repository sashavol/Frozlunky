#pragma once

#include "game_hooks.h"
#include <functional>
#include <vector>
#include <mutex>

class GameStateDetector {
public:
	typedef int bind_id;
	typedef std::function<void(int old_state, int new_state, bind_id)> bind_callback;

	struct bind_info {
		bind_callback callback;
		bool every;

		bind_info(decltype(callback) callback, decltype(every) every) : callback(callback), every(every) {}
		bind_info() : every(false) {}
	};

private:
	typedef std::map<bind_id, bind_info> binds_type;
	typedef std::map<int, binds_type> state_binds;

	std::shared_ptr<Spelunky> spel;
	state_binds binds;
	std::vector<bind_id> unbind_requests;
	std::mutex unbind_req_mu;
	Address game_state_offset;
	Address g_CurrentGamePtr;
	int last_state;
	bool is_valid;
	bind_id current_id;

	void real_unbind(bind_id id);

public:
	GameStateDetector(std::shared_ptr<DerandomizePatch> dp);

	bind_id bind(int state, bind_callback callback, bool every=false);
	void request_unbind(bind_id id);
	void cycle();
	int current_state();
};