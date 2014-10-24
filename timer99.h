#pragma once

#include "patches.h"
#include "derandom.h"
#include "spelunky.h"

class Timer99Patch : public Patch {
private:
	std::shared_ptr<DerandomizePatch> dp;
	std::shared_ptr<Spelunky> spel;
	bool is_valid;
	
	Address patch_addr;
	BYTE* orig;


	virtual bool _perform() override;
	virtual bool _undo() override;

public:
	Timer99Patch(std::shared_ptr<DerandomizePatch> dp);

	virtual bool valid() override;
};