#include "dark_possible.h"

BYTE dark_find[] = {0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x4E, 0x00, 0x00, 0x7D, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x74, 0x00, 0xC6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
std::string dark_mask = "x.....xxxxx.x.....x.x.....x";

BYTE check_patch[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};

DarkPossiblePatch::DarkPossiblePatch(std::shared_ptr<Spelunky> spel) : Patch(spel), is_valid(true), orig_check(nullptr) {
	dark_determine = spel->get_stored_hook("drkf");
	if(dark_determine == 0x0) {
		dark_determine = spel->find_mem(dark_find, dark_mask);
		if(dark_determine == 0x0) {
			is_valid = false;
			return;
		}
		dark_determine += 0x14;

#ifdef DEBUG_MODE
		std::cout << "Found dark_determine at " << std::setbase(16) << dark_determine << std::endl;
#endif

		spel->store_hook("drkf", dark_determine);
	}

	dark_check_pos = dark_determine-0x4C;
	orig_check = new BYTE[sizeof(check_patch)];
	spel->read_mem(dark_check_pos, orig_check, sizeof(check_patch));
}

DarkPossiblePatch::~DarkPossiblePatch() {
	if(orig_check != nullptr) {
		delete[] orig_check;
		orig_check = nullptr;
	}
}

bool DarkPossiblePatch::_perform() {
	spel->write_mem(dark_check_pos, check_patch, sizeof(check_patch)); 
	return true;
}

bool DarkPossiblePatch::_undo() {
	spel->write_mem(dark_check_pos, orig_check, sizeof(check_patch));
	return true;
}

