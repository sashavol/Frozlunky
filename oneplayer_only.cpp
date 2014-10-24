#include "oneplayer_only.h"

#ifdef DEBUG_MODE
#define DISCOVERY_FUNC(FUNC) if(!FUNC()) {std::cout << "OnePlayerOnlyPatch::" << #FUNC << " failed, hooks invalidated." << std::endl; is_valid = false; return;}
#else
#define DISCOVERY_FUNC(FUNC) if(!FUNC()) {is_valid = false; return;}
#endif

OnePlayerOnlyPatch::~OnePlayerOnlyPatch() {
	if(orig_char_select != nullptr) {
		delete[] orig_char_select;
	}
}

OnePlayerOnlyPatch::OnePlayerOnlyPatch(std::shared_ptr<Spelunky> spel, std::shared_ptr<DerandomizePatch> dp) : 
	Patch(spel), 
	is_valid(true),
	orig_char_select(nullptr)
{
	g_CurrentGamePtr = dp->game_ptr();
	if(g_CurrentGamePtr == 0x0) {
		is_valid = false;
		return;
	}

	DISCOVERY_FUNC(discover_controller_count);
	DISCOVERY_FUNC(discover_char_select_addr);
}

BYTE char_select_find[] = {0x7A, 0xFF, 
	0x6A, 0x01, 0xFF, 0xE8, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x6A, 0x02, 0xFF, 0xE8, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x6A, 0x03, 0xFF, 0xE8, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x6A, 0x04, 0xFF, 0xE8};

BYTE char_select_patch[] = {
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};

std::string char_select_mask = "x.xx.x....xx.x....xx.x....xx.x";

bool OnePlayerOnlyPatch::discover_char_select_addr() 
{
	char_select_addr = spel->get_stored_hook("csoffs");
	if(char_select_addr == 0x0) {
		Address base = spel->find_mem(char_select_find, char_select_mask);
		if(base == 0x0) {
			return false;
		}

		char_select_addr = base+10;

#ifdef DEBUG_MODE
		std::cout << "Discovered char_select_addr at " << std::setbase(16) << char_select_addr << std::endl;
#endif

		spel->store_hook("csoffs", char_select_addr);
	}

	orig_char_select = new BYTE[sizeof(char_select_patch)];
	spel->read_mem(char_select_addr, orig_char_select, sizeof(char_select_patch));

	return true;
}


BYTE controller_count_find[] = {0x83, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0F, 0x85, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x8B};
std::string controller_count_mask = "x.....xxx.....xxxxx";

bool OnePlayerOnlyPatch::discover_controller_count() 
{
	controller_count_offset = spel->get_stored_hook("ctcoffs");
	if(controller_count_offset == 0x0) {
		Address base = spel->find_mem(controller_count_find, controller_count_mask);
		if(base == 0x0) {
			return false;
		}

		base += 0x2;
		spel->read_mem(base, &controller_count_offset, sizeof(Address));

#ifdef DEBUG_MODE
		std::cout << "Discovered controller_count_offset at " << std::setbase(16) << controller_count_offset << std::endl;
#endif

		spel->store_hook("ctcoffs", controller_count_offset);
	}

	return true;
}

bool OnePlayerOnlyPatch::_perform() {
	if(char_select_addr != 0x0) {
		spel->write_mem(char_select_addr, char_select_patch, sizeof(char_select_patch));
	}

	return true;
}

bool OnePlayerOnlyPatch::_undo() {
	if(char_select_addr != 0x0) {
		spel->write_mem(char_select_addr, orig_char_select, sizeof(char_select_patch));
	}

	return true;
}

int OnePlayerOnlyPatch::controller_count() {
	Address game;
	spel->read_mem(g_CurrentGamePtr, &game, sizeof(Address));

	int count = 0;
	spel->read_mem(game+controller_count_offset, &count, sizeof(int));

	return count;
}