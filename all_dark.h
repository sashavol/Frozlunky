#pragma once

#include "patches.h"
#include "dark_possible.h"

class AllDarkPatch : public DarkPossiblePatch {
private:
	BYTE* no_dark_orig;
	Address no_dark_addr;

	virtual bool _perform() override;
	virtual bool _undo() override;

public:
	AllDarkPatch(std::shared_ptr<Spelunky> spel);
	~AllDarkPatch();
};