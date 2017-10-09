#include "temple_anticrash.h"

//+0
BYTE temple_anticrash_find[] = {0x74,0xCC,0x8B,0xCC,0xCC,0x80,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x75,0xCC,0x80,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x74};
std::string temple_anticrash_mask = "x.x..x......x.x......x";

BYTE temple_anticrash_repl[] = {0xEB};

//example crash seed = :foddev-taspel

TempleAnticrash::~TempleAnticrash() {
	if(orig) {
		delete[] orig;
	}
}

TempleAnticrash::TempleAnticrash(std::shared_ptr<Spelunky> spel) : Patch(spel),
	spel(spel),
	orig(nullptr),
	patch_addr(0x0),
	is_valid(true)
{
	patch_addr = spel->find_mem(temple_anticrash_find, temple_anticrash_mask);
	if(!patch_addr) {
		DBG_EXPR(std::cout << "[TempleAnticrash] Failed to find patch addr." << std::endl);
		is_valid = false;
		return;
	}
	DBG_EXPR(std::cout << "[TempleAnticrash] Patching at " << patch_addr << std::endl);

	orig = new BYTE[sizeof(temple_anticrash_repl)];
	spel->read_mem(patch_addr, orig, sizeof(temple_anticrash_repl));
}

bool TempleAnticrash::_perform() {
	spel->write_mem(patch_addr, temple_anticrash_repl, sizeof(temple_anticrash_repl));
	return true;
}

bool TempleAnticrash::_undo() {
	spel->write_mem(patch_addr, orig, sizeof(temple_anticrash_repl));
	return true;
}

bool TempleAnticrash::valid() {
	return is_valid;
}