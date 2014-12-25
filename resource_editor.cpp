#include "resource_editor.h"

//TODO health editing support

ResourceEditor::Resources::Resources() : bombs(-1), ropes(-1), health(-1) {}

void ResourceEditor::Resources::set(std::shared_ptr<GameHooks> gh, bool starting_level) {
	for(int p = 0; p < MAX_PLAYERS; ++p) {
		if(bombs > -1)
			gh->set_bombs(p, bombs);
		else if(starting_level)
			gh->set_bombs(p, RE_DEFAULT_BOMBS);

		if(ropes > -1)
			gh->set_ropes(p, ropes);
		else if(starting_level)
			gh->set_ropes(p, RE_DEFAULT_ROPES);
		
		if(health > -1)
			gh->set_health(p, health);
		else if(starting_level)
			gh->set_health(p, RE_DEFAULT_HEALTH);
	}
}

///////

ResourceEditor::ResourceEditor(std::shared_ptr<GameHooks> gh, std::shared_ptr<LevelRedirect> redirect, std::function<std::string()> level_getter, const std::vector<std::string>& areas) : 
	gh(gh), 
	redirect(redirect),
	last_level(""),
	level_getter(level_getter)
{
	for(auto&& area : areas) {
		resources[area] = Resources();
	}
}

void ResourceEditor::cycle() {
	int state = gh->game_state();

	if(state == STATE_INPUTLOCK_LEVELSTART || state == STATE_PLAYING) {
		std::string level = level_getter();
		bool starting_level = gh->current_level() == redirect->level_start;

		for(auto&& level_res : resources) {
			if(level == level_res.first && last_level != level_res.first) {
				level_res.second.set(gh, starting_level);
				last_level = level;
			}
		}
	}
	else if(state == STATE_LOBBY || state == STATE_GAMEOVER_HUD || state == STATE_MAINMENU) {
		last_level = "";
	}
}

void ResourceEditor::reset() {
	for(auto&& res : resources) {
		res.second = Resources();
	}
}

bool ResourceEditor::valid() {
	return true;
}

ResourceEditor::Resources& ResourceEditor::res(const std::string& area) {
	return resources[area];
}

std::map<std::string, ResourceEditor::Resources>::const_iterator ResourceEditor::begin() const {
	return resources.begin();
}

std::map<std::string, ResourceEditor::Resources>::const_iterator ResourceEditor::end() const {
	return resources.end();
}