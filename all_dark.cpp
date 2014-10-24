#include "all_dark.h"

BYTE no_dark_patch[] = {0x01};

AllDarkPatch::AllDarkPatch(std::shared_ptr<Spelunky> spel) : DarkPossiblePatch(spel), no_dark_orig(nullptr) {
	if(!DarkPossiblePatch::valid()) {
		return;
	}

	no_dark_addr = this->dark_determine_addr()+0x16;

	no_dark_orig = new BYTE[sizeof(no_dark_patch)];
	spel->read_mem(no_dark_addr, no_dark_orig, sizeof(no_dark_patch));
}

AllDarkPatch::~AllDarkPatch() {
	if(no_dark_orig != nullptr) {
		delete[] no_dark_orig;
		no_dark_orig = nullptr;
	}
}

bool AllDarkPatch::_perform() {
	spel->write_mem(no_dark_addr, no_dark_patch, sizeof(no_dark_patch));
	return true;
}

bool AllDarkPatch::_undo() {
	spel->write_mem(no_dark_addr, no_dark_orig, sizeof(no_dark_patch));
	return true;
}

