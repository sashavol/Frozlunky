#include "timer99.h"

std::string timer99_mask = "x..x.xxxxx....x";
BYTE timer99_find[] = {0x8D,0xCC,0xCC,0x69,0xCC,0xE8,0x03,0x00,0x00,0x3D,0xCC,0xCC,0xCC,0xCC,0x7C};
BYTE timer99_patch[] = {0xC3, 0x00};

Timer99Patch::Timer99Patch(std::shared_ptr<DerandomizePatch> dp) : Patch(dp->spel), 
	spel(dp->spel), 
	is_valid(true), 
	orig(nullptr) 
{
	Address timer99_addr = spel->find_mem(timer99_find, timer99_mask);
	if(timer99_addr > 0) {
		timer99_addr += 0xE;
		patch_addr = timer99_addr;

#ifdef DEBUG_MODE
		std::cout << "Found timer99_addr at " << std::setbase(16) << timer99_addr << std::endl;
#endif

		orig = new BYTE[sizeof(timer99_patch)];
		spel->read_mem(patch_addr, orig, sizeof(timer99_patch));
	}
	else {
#ifdef DEBUG_MODE
		std::cout << "Failed to find timer99_addr" << std::endl;
#endif

		is_valid = false;
	}
}

bool Timer99Patch::_perform() {
	spel->write_mem(patch_addr, timer99_patch, sizeof(timer99_patch));
	return true;
}

bool Timer99Patch::_undo() {
	spel->write_mem(patch_addr, orig, sizeof(timer99_patch));
	return true;
}

bool Timer99Patch::valid() {
	return is_valid;
}