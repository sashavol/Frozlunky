#pragma once

#include "patches.h"

class YetiRemovePatch : public Patch {
private:
	Address modif_addr;
	BYTE* orig;
	bool is_valid;

public:
	~YetiRemovePatch();
	YetiRemovePatch(std::shared_ptr<Spelunky> spel);

private:
	bool _perform() override;
	bool _undo() override;

public:
	bool valid() override;
};