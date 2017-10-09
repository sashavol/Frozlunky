#pragma once

#include "patches.h"

class AntiAchievementPatch : public Patch {
private:
	bool have_hook;
	Address achiev_fn;

public:
	AntiAchievementPatch(std::shared_ptr<Spelunky> spel);

	virtual bool valid() override;

private:
	virtual bool _perform() override;
	virtual bool _undo() override;
};