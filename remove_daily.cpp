#include "remove_daily.h"

std::string daily_patch_mask = ".x.x.....xx.....xx.x";
BYTE daily_patch_find[] = {0xFF, 0x8B, 0xFF, 0xC6, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0xC6, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x85};
BYTE daily_patch_offs_11_disable[] = {0x90, 0x90};

bool RemoveDailyPatch::_perform() {
	if(!daily_enabled) {
		if(daily_o11 != 0) {
			spel->write_mem(daily_o11, daily_patch_offs_11_disable, sizeof(daily_patch_offs_11_disable));
		}
	}
	else {
		//OPT
	}

	return true;
}

bool RemoveDailyPatch::_undo() {
	//spel->write_mem(daily_o11, original_daily_11, sizeof(daily_patch_offs_11_disable));
	return true;
}

RemoveDailyPatch::~RemoveDailyPatch() {
	delete[] original_daily_11;
}

RemoveDailyPatch::RemoveDailyPatch(std::shared_ptr<Spelunky> spel) : 
	Patch(spel), 
	is_valid(true), 
	daily_enabled(false), 
	daily_o11(0) 
{
	Address daily = spel->find_mem(daily_patch_find, daily_patch_mask);
	if(daily != 0) 
	{
		daily_o11 = daily+0x11;

		BYTE* daily_o11_orig = new BYTE[sizeof(daily_patch_offs_11_disable)];
		spel->read_mem(daily_o11, daily_o11_orig, sizeof(daily_patch_offs_11_disable));
		original_daily_11 = daily_o11_orig;

#ifdef DEBUG_MODE
		std::cout << "Found daily+11 patch at " << std::setbase(16) << daily_o11 << std::endl;
#endif
	}
	else 
	{
		is_valid = false;
#ifdef DEBUG_MODE
		std::cout << "Warning: remove daily patch was not able to locate offsets." << std::endl;
#endif
	}
}

void RemoveDailyPatch::set_daily_enabled(bool enabled) {
	if(!is_valid)
		return;

	daily_enabled = enabled;
	this->perform();
}