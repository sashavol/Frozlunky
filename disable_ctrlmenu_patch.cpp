#include "disable_ctrlmenu_patch.h"

BYTE controls_menu_find[] = {0x75,0xCC,0xCC,0xE8,0xCC,0xCC,0xCC,0xCC,0x83,0xCC,0xCC,0xCC,0xCC,0xCC,0x00,0x75,0xCC,0xCC,0xE8};
std::string controls_menu_mask = "x..x....x.....xx..x";

BYTE controls_menu_override[] = {0xEB};

DisableCtrlMenuPatch::DisableCtrlMenuPatch(std::shared_ptr<Spelunky> spel) : 
	Patch(spel), 
	spel(spel), 
	is_valid(true),
	menu_addr(0),
	orig(nullptr)
{
	menu_addr = spel->find_mem(controls_menu_find, controls_menu_mask);
	if(!menu_addr) {
		DBG_EXPR(std::cout << "[DisableCtrlMenuPatch] Failed to find menu_addr" << std::endl);
		is_valid = false;
		return;
	}
	DBG_EXPR(std::cout << "[DisableCtrlMenuPatch] Found menu_addr at " << menu_addr << std::endl);

	orig = new BYTE[sizeof(controls_menu_override)];
	spel->read_mem(menu_addr, orig, sizeof(controls_menu_override));
}

DisableCtrlMenuPatch::~DisableCtrlMenuPatch() {
	if(orig) {
		delete[] orig;
		orig = nullptr;
	}
}

bool DisableCtrlMenuPatch::_perform() {
	spel->write_mem(menu_addr, controls_menu_override, sizeof(controls_menu_override));
	return true;
}

bool DisableCtrlMenuPatch::_undo() {
	spel->write_mem(menu_addr, orig, sizeof(controls_menu_override));
	return true;
}

bool DisableCtrlMenuPatch::valid() {
	return is_valid;
}