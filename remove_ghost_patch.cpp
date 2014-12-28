#include "remove_ghost_patch.h"

//+0
static BYTE ghost_find[] = {0x7C,0xCC,0x80,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x75,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0x83};
static std::string ghost_mask = "x.x......x.x.....x";

static BYTE opcode_overwrite[] = {0xEB};

RemoveGhostPatch::RemoveGhostPatch(std::shared_ptr<GameHooks> gh) : Patch(gh->spel),
	is_valid(true),
	patch_addr(0),
	orig(nullptr)
{
	patch_addr = spel->find_mem(ghost_find, ghost_mask);
	if(!patch_addr) {
		is_valid = false;
		DBG_EXPR(std::cout << "[RemoveGhostPatch] Failed to find patch_addr" << std::endl);
		return;
	}
	DBG_EXPR(std::cout << "[RemoveGhostPatch] Found patch_addr = " << patch_addr << std::endl);

	orig = new BYTE[sizeof(opcode_overwrite)];
	spel->read_mem(patch_addr, orig, sizeof(opcode_overwrite));
}

RemoveGhostPatch::~RemoveGhostPatch() {
	if(orig) {
		delete[] orig;
		orig = nullptr;
	}
}

bool RemoveGhostPatch::_perform() {
	spel->write_mem(patch_addr, opcode_overwrite, sizeof(opcode_overwrite));
	return true;
}

bool RemoveGhostPatch::_undo() {
	spel->write_mem(patch_addr, orig, sizeof(opcode_overwrite));
	return true;
}

bool RemoveGhostPatch::valid() {
	return is_valid;
}