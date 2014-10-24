#pragma once

#include "spelunky.h"
#include "patches.h"

#include <map>
#include <string>
#include <vector>

class PatchGroup : public Patch {
	std::map<std::string, std::shared_ptr<Patch>> patches;
	std::vector<std::map<std::string, bool>> patch_states;

	virtual bool _perform() override;
	virtual bool _undo() override;

public:
	PatchGroup(std::shared_ptr<Spelunky> spel) : Patch(spel) {}
	PatchGroup(std::shared_ptr<Spelunky> spel, const std::map<std::string, std::shared_ptr<Patch>>& patches) : Patch(spel), patches(patches) {}

	virtual bool valid() override;

	virtual void lock() override;
	virtual void unlock() override;

	PatchGroup* add(const std::string& name, std::shared_ptr<Patch> patch);
	std::shared_ptr<Patch> get(const std::string& name);

	void push_state();
	void pop_state();
	void pop_noload_state();
};