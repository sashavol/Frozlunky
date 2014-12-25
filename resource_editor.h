#pragma once

#include "derandom.h"
#include "game_hooks.h"
#include "level_forcer.h"
#include <map>
#include <string>
#include <functional>

#define RE_DEFAULT_BOMBS 4
#define RE_DEFAULT_ROPES 4
#define RE_DEFAULT_HEALTH 4

#define MIN_BOMBS_VAL -1
#define MIN_ROPES_VAL -1
#define MIN_HEALTH_VAL -1

#define MAX_BOMBS_VAL 99
#define MAX_ROPES_VAL 99
#define MAX_HEALTH_VAL 99

//modifies starting resources
class ResourceEditor {
public:
	struct Resources {
		int bombs;
		int ropes;
		int health;

		Resources();
		
		void set(std::shared_ptr<GameHooks> gh, bool starting_level);
	};

private:
	std::map<std::string, Resources> resources;

	std::function<std::string()> level_getter;
	std::string last_level;
	
	std::shared_ptr<LevelRedirect> redirect;
	std::shared_ptr<GameHooks> gh;
	std::shared_ptr<Spelunky> spel;

public:
	ResourceEditor(std::shared_ptr<GameHooks> gh, std::shared_ptr<LevelRedirect> redirect, std::function<std::string()> level_getter, const std::vector<std::string>&  areas);

	Resources& res(const std::string& area);
	std::map<std::string, Resources>::const_iterator begin() const;
	std::map<std::string, Resources>::const_iterator end() const;

	void reset();

	void cycle();
	bool valid();
};