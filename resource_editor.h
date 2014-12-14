#pragma once

#include "derandom.h"
#include "game_hooks.h"
#include <map>

#define RE_DEFAULT_BOMBS 4
#define RE_DEFAULT_ROPES 4
#define RE_DEFAULT_HEALTH 4

//modifies starting resources
class ResourceEditor {
public:
	struct Resources {
		int bombs;
		int ropes;
		int health;

		Resources();
		
		void set(std::shared_ptr<GameHooks> gh);
	};

private:
	std::map<int, Resources> resources;

	int last_level;
	std::shared_ptr<GameHooks> gh;
	std::shared_ptr<Spelunky> spel;

public:
	ResourceEditor(std::shared_ptr<GameHooks> gh);

	void cycle();
	bool valid();
};