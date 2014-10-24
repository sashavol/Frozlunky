#pragma once

#include "patches.h"

class DisableCtrlMenuPatch : public Patch {
private:
	std::shared_ptr<Spelunky> spel;
	Address menu_addr;
	BYTE* orig;

	bool is_valid;

	bool _perform() override;
	bool _undo() override;

public:
	~DisableCtrlMenuPatch();
	DisableCtrlMenuPatch(std::shared_ptr<Spelunky> spel);

	bool valid() override;
};