#include "antipause_patch.h"
#include "debug.h"

//+0
BYTE antipause_find[] = {0xE8,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xFF,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xC2};
std::string antipause_mask = "x....x...x.......x.......x";

BYTE antipause_override[] = {0x90, 0x90, 0x90, 0x90, 0x90};

AntipausePatch::AntipausePatch(std::shared_ptr<Spelunky> spel) : Patch(spel), is_valid(true), orig(nullptr)
{
	antipause_target = spel->get_stored_hook("antipause");
	if(!antipause_target) {
		antipause_target = spel->find_mem(antipause_find, antipause_mask);
		if(!antipause_target) {
			DBG_EXPR(std::cout << "[AntipausePatch] Failed to find antipause target override." << std::endl);
			is_valid = false;
			return;
		}
		DBG_EXPR(std::cout << "[AntipausePatch] Overriding at " << antipause_target << std::endl);
		spel->store_hook("antipause", antipause_target);
	}

	orig = new BYTE[sizeof(antipause_override)];
	spel->read_mem(antipause_target, orig, sizeof(antipause_override));
}

AntipausePatch::~AntipausePatch() {
	if(orig) {
		delete[] orig;
		orig = nullptr;
	}
}

bool AntipausePatch::valid() {
	return is_valid;
}

bool AntipausePatch::_perform() {
	spel->write_mem(antipause_target, antipause_override, sizeof(antipause_override));
	return true;
}

bool AntipausePatch::_undo() {
	spel->write_mem(antipause_target, orig, sizeof(antipause_override));
	return true;
}