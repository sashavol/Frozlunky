#include "yeti_remove_patch.h"

//+9
static BYTE sub_find[] = {0x74,0xCC,0x80,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x74,0xCC,0x8D,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0x8B,0xCC,0x8B};
static std::string sub_mask = "x.x......x.x....x.x.x";

static BYTE sub_mod[] = {0xEB}; //jmp

YetiRemovePatch::~YetiRemovePatch() {
	if(orig) {
		delete[] orig;
		orig = nullptr;
	}
}

YetiRemovePatch::YetiRemovePatch(std::shared_ptr<Spelunky> spel) : 
	Patch(spel),
	orig(nullptr),
	modif_addr(0),
	is_valid(true)
{
	Address sub = spel->find_mem(sub_find, sub_mask);
	if(!sub) {
		is_valid = false;
		DBG_EXPR(std::cout << "[YetiRemovePatch] Failed to find subroutine." << std::endl);
		return;
	}
	modif_addr = sub + 9;
	DBG_EXPR(std::cout << "[YetiRemovePatch] Applying patch at " << modif_addr << std::endl);

	orig = new BYTE[sizeof(sub_mod)];
	spel->read_mem(modif_addr, orig, sizeof(sub_mod));
}

bool YetiRemovePatch::_perform() {
	spel->write_mem(modif_addr, sub_mod, sizeof(sub_mod));
	return true;
}

bool YetiRemovePatch::_undo() {
	spel->write_mem(modif_addr, orig, sizeof(sub_mod));
	return true;
}

bool YetiRemovePatch::valid() {
	return is_valid;
}