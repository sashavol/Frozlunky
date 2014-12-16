#pragma once

#include "derandom.h"
#include "game_hooks.h"
#include <map>
#include <string>
#include <functional>

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
	std::map<std::string, Resources> resources;

	std::function<std::string()> level_getter;
	std::string last_level;
	
	std::shared_ptr<GameHooks> gh;
	std::shared_ptr<Spelunky> spel;

public:
	ResourceEditor(std::shared_ptr<GameHooks> gh, std::function<std::string()> level_getter, const std::vector<std::string>&  areas);

	Resources& res(const std::string& area);

	void cycle();
	bool valid();
};