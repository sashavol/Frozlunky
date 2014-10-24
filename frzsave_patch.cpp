#include "frzsave_patch.h"

BYTE savefile_find[] =		"spelunky_save";
std::string savefile_mask = "xxxxxxxxxxxxx";
BYTE savefile_override[] =  "frzlunky_save";

//TODO force load LoadSavefile<eax>(int game<esi>) in preconditions.cpp
//TODO (not in this context) Fix special patches not being undone when speed daily is started

FrzSavePatch::FrzSavePatch(std::shared_ptr<Spelunky> spel) : 
	Patch(spel),
	is_valid(true)
{
	Address p = 0;
	while(p = spel->find_mem(savefile_find, savefile_mask, p)) {
		patch_addrs.push_back(p);
		p++;
	}
	
	if(patch_addrs.size() == 0) {
		DBG_EXPR(std::cout << "[FrzSavePatch] Failed to find spelunky_save.*" << std::endl);
		is_valid = false;
		return;
	}
	else {
		DBG_EXPR(std::cout << "[FrzSavePatch] Patching " << patch_addrs.size() << " instances of spelunky_save.* => frzlunky_save.*" << std::endl);
		DBG_EXPR(
			std::cout << "[FrzSavePatch] [ ";
			for(Address addr : patch_addrs) {
				std::cout << addr << " ";
			}
			std::cout << "]" << std::endl;
		);
	}
}

bool FrzSavePatch::valid() {
	return is_valid;
}

bool FrzSavePatch::_perform() {
	for(Address addr : patch_addrs) {
		spel->write_mem(addr, savefile_override, sizeof(savefile_override) - 1, true);
	}
	return true;
}

bool FrzSavePatch::_undo() {
	//permanent
	//for(Address addr : patch_addrs) {
	//	spel->write_mem(addr, savefile_find, sizeof(savefile_find) - 1, true);
	//}
	return true;
}