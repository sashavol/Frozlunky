#pragma once

#include "patches.h"

class DarkPossiblePatch : public Patch {
private:
	bool is_valid;
	Address dark_determine;
	Address dark_check_pos;
	BYTE* orig_check;

	virtual bool _perform() override;
	virtual bool _undo() override;

public:
	DarkPossiblePatch(std::shared_ptr<Spelunky> spel);
	~DarkPossiblePatch();

	virtual bool valid() override {
		return is_valid;
	}

	Address dark_determine_addr() {
		return dark_determine;
	}
};