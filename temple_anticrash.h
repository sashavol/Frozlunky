#pragma once

#include "patches.h"

class TempleAnticrash : public Patch {
private:
	std::shared_ptr<Spelunky> spel;
	bool is_valid;

	Address patch_addr;
	BYTE* orig;

	virtual bool _perform() override;
	virtual bool _undo() override;

public:
	~TempleAnticrash();
	TempleAnticrash(std::shared_ptr<Spelunky> spel);

	virtual bool valid() override;
};