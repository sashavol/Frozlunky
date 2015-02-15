#include "message_grid.h"

MessageGrid::MessageGrid(std::shared_ptr<GameHooks> gh) : 
	gh(gh) 
{}

MessageGrid::grid::iterator MessageGrid::find(const cc_pos& p) {
	return cc_grid.find(p);
}

MessageGrid::cc_pos MessageGrid::player_pos(int idx) {
	float x = gh->player_x(idx) + 0.5f, y = gh->player_y(idx);
	return std::make_pair(int(x - 3.0f), int(99.0f - y));
}

MessageGrid::grid::iterator MessageGrid::player_message() {
	for(int i = 0; i < MAX_PLAYERS; ++i) {
		grid::iterator it = find(player_pos(i));
		if(it != end()) {
			return it;
		}
	}
	return end();
}

MessageGrid::grid::iterator MessageGrid::end() {
	return cc_grid.end();
}

void MessageGrid::insert(cc_pos p, const std::string& message) {
	cc_grid[p] = message;
}

void MessageGrid::erase(grid::iterator it) {
	cc_grid.erase(it);
}

void MessageGrid::clear() {
	cc_grid.clear();
}