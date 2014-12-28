#pragma once

#include "patches.h"
#include "game_hooks.h"

class RemoveGhostPatch : public Patch {
private:
	bool is_valid;
	Address patch_addr;
	BYTE* orig;

public:
	RemoveGhostPatch(std::shared_ptr<GameHooks> gh);
	~RemoveGhostPatch();

private:
	virtual bool _perform() override;
	virtual bool _undo() override;

public:
	virtual bool valid() override;
};