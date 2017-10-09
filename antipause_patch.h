#pragma once

#include "patches.h"
#include "spelunky.h"

//stops pausing when the user defocuses
class AntipausePatch : public Patch {
private:
	bool is_valid;

	BYTE* orig;
	Address antipause_target;

public:
	AntipausePatch(std::shared_ptr<Spelunky> spel);
	~AntipausePatch();

	virtual bool valid() override;

private:
	virtual bool _perform() override;
	virtual bool _undo() override;
};