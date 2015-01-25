#pragma once

#include "patches.h"
#include "game_hooks.h"

class AntispawnPatch : public Patch {
private:
	std::shared_ptr<GameHooks> gh;

	Address anubis;
	BYTE* orig_anubis;
	
	Address damsel;
	BYTE* orig_damsel;

	Address tiki;
	BYTE* orig_tiki;

	Address key_chest;
	BYTE* orig_key_chest;

	Address gems;
	BYTE* orig_gems;

	Address worm_tongue;
	BYTE* orig_worm_tongue;

	bool is_valid;

public:
	~AntispawnPatch();
	AntispawnPatch(std::shared_ptr<GameHooks> gh);

private:
	virtual bool _perform() override;
	virtual bool _undo() override;

public:
	virtual bool valid() override;
};