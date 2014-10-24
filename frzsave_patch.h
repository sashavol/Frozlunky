#pragma once

#include "patches.h"
#include <vector>

//reconfigures Spelunky to save/load frzlunky_save.dat instead of spelunky_save.dat
class FrzSavePatch : public Patch {
private:
	bool is_valid;
	std::vector<Address> patch_addrs;

	virtual bool _perform() override;
	virtual bool _undo() override;

public:
	FrzSavePatch(std::shared_ptr<Spelunky> spel);

	virtual bool valid() override;
};