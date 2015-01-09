#include "protect_achiev.h"
#include <iomanip>

//+0
static BYTE achiev_find[] = {0x0F,0x84,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x68,0xCC,0xCC,0xCC,0xCC,0xFF,0xCC,0xCC,0xCC,0xCC,0xCC,0xBB};
static std::string achiev_mask = "xx......x....x.....x";

//nop first byte + set to jmp
static BYTE achiev_patch[] = {0x90, 0xE9};

AntiAchievementPatch::AntiAchievementPatch(std::shared_ptr<Spelunky> spel) : 
	Patch(spel), 
	have_hook(true),
	achiev_fn(0)
{
	achiev_fn = spel->find_mem(achiev_find, achiev_mask);

	if(!achiev_fn) {
		DBG_EXPR(std::cout << "[AntiAchievementPatch] Failed to find achiev_fn " << std::endl);
		have_hook = false;
		return;
	}

	DBG_EXPR(std::cout << std::setbase(16) << "[AntiAchievementPatch] Found achiev_fn at " << achiev_fn << std::setbase(10) << std::endl);
}

//always state validity is true.
//if the patch is unable to find hooks, it will bypass patching to support GoG.
bool AntiAchievementPatch::valid() {
	return true;
}

bool AntiAchievementPatch::_perform() {
	if(have_hook) {
		spel->write_mem(achiev_fn, achiev_patch, sizeof(achiev_patch));
	}
	return true;
}

bool AntiAchievementPatch::_undo() {
	//do not undo
	return true;
}