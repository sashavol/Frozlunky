#include "resource_editor.h"

//TODO health editing support

ResourceEditor::Resources::Resources() : bombs(-1), ropes(-1), health(-1) {}

void ResourceEditor::Resources::set(std::shared_ptr<GameHooks> gh) {
	for(int p = 0; p < MAX_PLAYERS; ++p) {
		if(bombs > -1)
			gh->set_bombs(p, bombs);
		if(ropes > -1)
			gh->set_ropes(p, ropes);
		if(health > -1)
			gh->set_health(p, health);
	}
}

///////

ResourceEditor::ResourceEditor(std::shared_ptr<GameHooks> gh, std::function<std::string()> level_getter, const std::vector<std::string>& areas) : 
	gh(gh), 
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
		for(auto&& level_res : resources) {
			if(level == level_res.first && last_level != level_res.first) {
				level_res.second.set(gh);
				last_level = level;
			}
		}
	}
	else if(state == STATE_LOBBY || state == STATE_GAMEOVER_HUD) {
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