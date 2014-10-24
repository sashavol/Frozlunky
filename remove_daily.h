#pragma once

#include "patches.h"

class RemoveDailyPatch : public Patch {
private:
	BYTE* original_daily_11;
	Address daily_o11;

	bool is_valid;
	bool daily_enabled;

private:
	virtual bool _perform() override;
	virtual bool _undo() override;

public:
	~RemoveDailyPatch();
	RemoveDailyPatch(std::shared_ptr<Spelunky> spel);

	void set_daily_enabled(bool enabled);
	virtual bool valid() override {
		return is_valid;
	}
};