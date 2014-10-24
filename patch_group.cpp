#include "patch_group.h"

bool PatchGroup::_perform() {
	for(std::pair<std::string, std::shared_ptr<Patch>> patch : patches) {
		patch.second->perform();
		if(patch.second->valid() && !patch.second->is_active()) {
#ifdef DEBUG_MODE
			std::cout << "Warning: patch \"" << patch.first << "\" failed to be activated." << std::endl;
#endif

			return false;
		}
	}
	return true;
}

bool PatchGroup::_undo() {
	for(std::pair<std::string, std::shared_ptr<Patch>> patch : patches) {
		patch.second->undo();
		if(patch.second->valid() && patch.second->is_active()) {
#ifdef DEBUG_MODE
			std::cout << "Warning: patch \"" << patch.first << "\" failed to be reversed." << std::endl;
#endif

			return false;
		}
	}
	return true;
}

bool PatchGroup::valid() {
	for(std::pair<std::string, std::shared_ptr<Patch>> patch : patches) {
		if(patch.second->valid())
			return true;
	}
	return true;
}

void PatchGroup::lock() {
	Patch::lock();
	for(auto iter = patches.begin(), end = patches.end(); iter != end; iter++) {
		iter->second->lock();
	}
}

void PatchGroup::unlock() {
	Patch::unlock();
	for(auto iter = patches.begin(), end = patches.end(); iter != end; iter++) {
		iter->second->unlock();
	}
}

PatchGroup* PatchGroup::add(const std::string& name, std::shared_ptr<Patch> patch) {
	if(patches.find(name) == patches.end()) {
		patches[name] = patch;
	}
	return this;
}

std::shared_ptr<Patch> PatchGroup::get(const std::string& name) {
	auto iter = patches.find(name);
	if(iter == patches.end())
		return nullptr;
	else
		return iter->second;
}

void PatchGroup::push_state() {
	std::map<std::string, bool> state;
	for(std::pair<std::string, std::shared_ptr<Patch>> patch : patches) {
		state[patch.first] = patch.second->is_active();
	}
	patch_states.push_back(state);
}

void PatchGroup::pop_state() {
	if(patch_states.size() > 0) {
		std::map<std::string, bool>& state = patch_states.back();

		for(auto iter = state.begin(); iter != state.end(); iter++) {
			auto physical = patches.find(iter->first);
			if(physical != patches.end()) {
				if(iter->second) {
					if(!physical->second->is_active()) {
						physical->second->perform();
					}
				}
				else if(physical->second->is_active()) {
					physical->second->undo();
				}
			}
		}

		patch_states.pop_back();	
	}
}

void PatchGroup::pop_noload_state() {
	if(patch_states.size() > 0) {
		patch_states.pop_back();
	}
}
