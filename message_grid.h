#pragma once

#include <string>
#include <map>
#include "game_hooks.h"

struct MessageGrid {
	typedef std::pair<int, int> cc_pos;
	typedef std::map<cc_pos, std::string> grid;
	
private:
	std::shared_ptr<GameHooks> gh;

private:
	cc_pos player_pos(int idx);

public:
	grid cc_grid;

	MessageGrid(std::shared_ptr<GameHooks> gh);

	grid::iterator find(const cc_pos& p);
	grid::iterator player_message();
	grid::iterator end();

	void insert(cc_pos p, const std::string& message);
	void erase(grid::iterator it);
	void clear();
};