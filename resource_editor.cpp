#include "resource_editor.h"

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

ResourceEditor::ResourceEditor(std::shared_ptr<GameHooks> gh) : gh(gh), last_level(-1) {
	for(int lvl = LEVEL_1_1; lvl <= LEVEL_5_4; ++lvl) {
		resources[lvl] = Resources();
	}
}

void ResourceEditor::cycle() {
	int state = gh->game_state();
	if(state == STATE_PLAYING || state == STATE_INPUTLOCK_LEVELSTART) {
		for(auto&& level_res : resources) {
			if(last_level == level_res.first && last_level != level_res.first) {
				level_res.second.set(gh);
				last_level = level_res.first;
			}
		}
	}
	else if(state == STATE_LOBBY || state == STATE_GAMEOVER_HUD) {
		last_level = -1;
	}
}

bool ResourceEditor::valid() {
	return true;
}