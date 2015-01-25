#include "antispawn.h"

//+0
static BYTE key_chest_find[] = {0x0F,0xCC,0xCC,0xCC,0xCC,0xCC,0xE8,0xCC,0xCC,0xCC,0xCC,0x33,0xCC,0xF7,0xCC,0x83,0xCC,0xCC,0x8B};
static std::string key_chest_mask = "x.....x....x.x.x..x";
static BYTE key_chest_override[] = {0x90, 0xE9};

//+0
static BYTE tiki_find[] = {0x0F,0x8E,0xCC,0xCC,0xCC,0xCC,0xCC,0x89,0xCC,0xCC,0xCC,0xEB,0xCC,0xB9,0xCC,0xCC,0xCC,0xCC,0x39};
static std::string tiki_mask = "xx.....x...x.x....x";
static BYTE tiki_override[] = {0x90, 0xE9};

//+0
static BYTE damsel_find[] = {0x0F,0x8E,0xCC,0xCC,0xCC,0xCC,0x8D,0xCC,0xCC,0xCC,0xE8,0xCC,0xCC,0xCC,0xCC,0x33,0xCC,0xF7};
static std::string damsel_mask = "xx....x...x....x.x";
static BYTE damsel_override[] = {0x90, 0xE9};

//+0
static BYTE anubis_find[] = {0x0F,0x85,0xCC,0xCC,0xCC,0xCC,0x83,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x75,0xCC,0x39,0xCC,0xCC,0xCC,0x7E};
static std::string anubis_mask = "xx....x......x.x...x";
static BYTE anubis_override[] = {0x90, 0xE9};

//+0
static BYTE gems_find[] = {0x0F,0x85,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0x8D,0xCC,0xCC,0x85,0xCC,0x0F,0x8C};
static std::string gems_mask = "xx....x...x..x.xx";
static BYTE gems_override[] = {0x90,0xE9};

//+0
static BYTE worm_tongue_find[] = {0x0F,0x85,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0x83,0xCC,0xCC,0x74,0xCC,0x83,0xCC,0xCC,0x0F,0x85};
static std::string worm_tongue_mask = "xx....x.....x..x.x..xx";
static BYTE worm_tongue_override[] = {0x90, 0xE9};

AntispawnPatch::~AntispawnPatch() {
	if(orig_anubis)
		delete[] orig_anubis;
	if(orig_damsel)
		delete[] orig_damsel;
	if(orig_key_chest)
		delete[] orig_key_chest;
	if(orig_tiki)
		delete[] orig_tiki;
	if(orig_gems)
		delete[] orig_gems;
	if(orig_worm_tongue)
		delete[] orig_worm_tongue;
}

#define DISCOVER(type, offs) { \
	type = spel->find_mem(type##_find, type##_mask); \
	if(!type) { \
		DBG_EXPR(std::cout << "[AntispawnPatch] Failed to find " #type << std::endl);\
		is_valid = false; \
		return; \
	} \
	type += offs; \
	DBG_EXPR(std::cout << "[AntispawnPatch] Found " #type " at " << type << std::endl); \
	orig_##type = new BYTE[sizeof(type##_override)]; \
	spel->read_mem(type, orig_##type, sizeof(type##_override)); \
}

AntispawnPatch::AntispawnPatch(std::shared_ptr<GameHooks> gh) : Patch(gh->spel), 
	gh(gh),
	orig_anubis(nullptr),
	orig_damsel(nullptr),
	orig_key_chest(nullptr),
	orig_tiki(nullptr),
	orig_gems(nullptr),
	orig_worm_tongue(nullptr),
	anubis(0),
	damsel(0),
	tiki(0),
	key_chest(0),
	gems(0),
	is_valid(true)
{
	DISCOVER(key_chest, 0);
	DISCOVER(tiki, 0);
	DISCOVER(damsel, 0);
	DISCOVER(anubis, 0);
	DISCOVER(gems, 0);
	DISCOVER(worm_tongue, 0);
}

bool AntispawnPatch::_perform() {
	spel->write_mem(key_chest, key_chest_override, sizeof(key_chest_override));
	spel->write_mem(tiki, tiki_override, sizeof(tiki_override));
	spel->write_mem(damsel, damsel_override, sizeof(damsel_override));
	spel->write_mem(anubis, anubis_override, sizeof(anubis_override));
	spel->write_mem(gems, gems_override, sizeof(gems_override));
	spel->write_mem(worm_tongue, worm_tongue_override, sizeof(worm_tongue_override));
	return true;
}

bool AntispawnPatch::_undo() {
	spel->write_mem(key_chest, orig_key_chest, sizeof(key_chest_override));
	spel->write_mem(tiki, orig_tiki, sizeof(tiki_override));
	spel->write_mem(damsel, orig_damsel, sizeof(damsel_override));
	spel->write_mem(anubis, orig_anubis, sizeof(anubis_override));
	spel->write_mem(gems, orig_gems, sizeof(gems_override));
	spel->write_mem(worm_tongue, orig_worm_tongue, sizeof(worm_tongue_override));
	return true;
}

bool AntispawnPatch::valid() {
	return is_valid;
}