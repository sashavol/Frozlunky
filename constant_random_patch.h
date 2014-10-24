#pragma once

#include "patches.h"
#include "derandom.h"
#include "game_hooks.h"
#include "seeder.h"

//Forces Random() to always return a single value during game time (state = 0)
class ConstantRandomPatch : public Patch {
private:
	std::shared_ptr<DerandomizePatch> dp;
	std::shared_ptr<GameHooks> gh;
	std::shared_ptr<Seeder> seeder;
	bool is_valid;

	Address random_fn;
	Address routine_alloc;

	BYTE* orig_random;

	virtual bool _perform() override;
	virtual bool _undo() override;

public:
	ConstantRandomPatch(std::shared_ptr<DerandomizePatch> dp, std::shared_ptr<GameHooks> gh, std::shared_ptr<Seeder> seeder);
	ConstantRandomPatch::~ConstantRandomPatch();

	virtual bool valid() override;
};