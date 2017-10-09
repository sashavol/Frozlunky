#pragma once

#include "patches.h"
#include "debug.h"
#include "protect_achiev.h"
#include <vector>


class OfflinePatch : public Patch {
private:
	std::shared_ptr<AntiAchievementPatch> anti_achiev;
	std::vector<Address> patched_addrs;
	std::vector<BYTE*> original_values;
	bool is_valid;

public:
	~OfflinePatch();
	OfflinePatch(std::shared_ptr<Spelunky> spel);
	
	virtual bool _undo() override;
	virtual bool _perform() override;

public:
	virtual bool valid() override;
};