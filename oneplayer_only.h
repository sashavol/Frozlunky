#pragma once

#include "patches.h"
#include "derandom.h"

class OnePlayerOnlyPatch : public Patch {
	bool is_valid;

	Address g_CurrentGamePtr;
	Address controller_count_offset;
	Address char_select_addr;

	BYTE* orig_char_select;

	virtual bool _perform() override;
	virtual bool _undo() override;

	bool discover_controller_count();
	bool discover_char_select_addr();

public:
	~OnePlayerOnlyPatch();
	OnePlayerOnlyPatch(std::shared_ptr<Spelunky> spel, std::shared_ptr<DerandomizePatch> dp);
	
	virtual bool valid() override {
		return is_valid;
	}

	int controller_count();
};