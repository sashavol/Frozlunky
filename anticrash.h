#pragma once

#include "patches.h"
#include "debug.h"

BYTE waterbubbles_patch_find[] = {0x56, 0x57, 0x8D, 0x44, 0x24, 0x18, 0xE8, 0x00, 0x00, 0x00, 0x00, 0xD9, 0xE8, 0x8B, 0x84, 0x24, 0x8C, 0x00, 0x00, 0x00};
const std::string waterbubbles_patch_mask = "xxxxxxx....xxxxxxxxx";

const std::string worm_patch_mask = "xxxxxxxxxxx....xxxxxxxxxxx";
BYTE worm_patch_find[] = {0xD9, 0xE8, 0x8B, 0x55, 0x34, 0xD9, 0x5C, 0x24, 0x28, 0xD9, 0x05, 0x00, 0x00, 0x00, 0x00, 0xBB, 0x01, 0x00, 0x00, 0x00, 0x23, 0xC3, 0xD9, 0x5C, 0x24, 0x2C};

const std::string olmec_patch_mask = "xxxxxxxxxx....xxxxxx....xxxxx";
BYTE olmec_patch_find[] = {0xD9, 0x5C, 0x24, 0x38, 0xD9, 0x5C, 0x24, 0x3C, 0xD9, 0x05, 0x00, 0x00, 0x00, 0x00, 0xD9, 0x5C, 0x24, 0x40, 0xD9, 0x05, 0x00, 0x00, 0x00, 0x00, 0xD9, 0x5C, 0x24, 0x44, 0xE8}; //TODO

BYTE waterbubbles_patch[] = {0x5F, 0x5E, 0x5D, 0x83, 0xC4, 0x74, 0xC2, 0x0C, 0x00};
BYTE worm_patch[] = {0x5F, 0x5E, 0x5D, 0x5B, 0x83, 0xC4, 0x74, 0xC2, 0x08, 0x00};
BYTE olmec_patch[] = {0x5F, 0x5E, 0x5D, 0x83, 0xC4, 0x70, 0xC2, 0x08, 0x00};

const std::string overflow_mask = "xxxxxxxxxxxxx";
BYTE overflow_find[] = {0x8B, 0x81, 0x10, 0x78, 0x00, 0x00, 0x3D, 0x00, 0x05, 0x00, 0x00, 0x7C, 0x0A};
BYTE overflow_patch[] = {0xB8, 0x00, 0x00, 0x00, 0x00, 0xC3, 0x90, 0x90, 0x90, 0x90};

const std::string overflow2_mask = "xxxxxxxxxxxxxxxx";
BYTE overflow2_find[] = {0xC6, 0x87, 0x95, 0x02, 0x00, 0x00, 0x01, 0x8B, 0x43, 0x30, 0x8B, 0x88, 0x10, 0x78, 0x00, 0x00};
BYTE overflow2_patch[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};

const std::string sparkles_mask = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
BYTE sparkles_find[] = {0xF7, 0xF1, 0xD9, 0x5C, 0x24, 0x24, 0xC6, 0x44, 0x24, 0x6A, 0x00, 0x42, 0x89, 0x54, 0x24, 0x14, 0x8B, 0x55, 0x34, 0x33, 0xC0, 0x8B, 0xCA, 0x81, 0x39, 0x2E, 0x01, 0x00, 0x00};
BYTE sparkles_patch[] = {0x5F, 0x5E, 0x5D, 0x83, 0xC4, 0x70, 0xC2, 0x08, 0x00};

class AnticrashPatch : public Patch {
private:
	bool is_valid;
	Address waterbubbles_patch_addr;
	Address worm_patch_addr;
	Address olmec_patch_addr;
	Address sparkles_patch_addr;

	BYTE* orig_waterbubbles;
	BYTE* orig_worm;
	BYTE* orig_olmec;
	BYTE* orig_sparkles;

	Address entity_overflow_addr;
	Address entity_overflow2_addr;

public:
	~AnticrashPatch() {
		if(orig_waterbubbles != nullptr) {
			delete[] orig_waterbubbles;
			orig_waterbubbles = nullptr;
		}

		if(orig_worm != nullptr) {
			delete[] orig_worm;
			orig_worm = nullptr;
		}

		//TODO the other patches
	}

	AnticrashPatch(std::shared_ptr<Spelunky> spel) : Patch(spel), 
		is_valid(true), 
		orig_worm(nullptr), 
		orig_waterbubbles(nullptr),
		orig_olmec(nullptr)
	{//
		{
			entity_overflow_addr = spel->find_mem(overflow_find, overflow_mask)+0xD;

#ifdef DEBUG_MODE
			std::cout << "Entity overflow patch at " << std::setbase(16) << entity_overflow_addr << std::endl;
#endif
		}

		{
			entity_overflow2_addr = spel->find_mem(overflow2_find, overflow2_mask)+0x18;

#ifdef DEBUG_MODE
			std::cout << "Entity overflow 2 patch at " << std::setbase(16) << entity_overflow2_addr << std::endl;
#endif
		}

		{
			waterbubbles_patch_addr = spel->find_mem(waterbubbles_patch_find, waterbubbles_patch_mask)+0xB;
			if(waterbubbles_patch_addr != 0) {
	#ifdef DEBUG_MODE
				std::cout << "Anticrash patch at " << std::setbase(16) << waterbubbles_patch_addr << std::endl;
	#endif

				orig_waterbubbles = new BYTE[sizeof(waterbubbles_patch)];
				spel->read_mem(waterbubbles_patch_addr, orig_waterbubbles, sizeof(waterbubbles_patch));
			}
			else {
				is_valid = false;
			}
		}

		{
			worm_patch_addr = spel->find_mem(worm_patch_find, worm_patch_mask);
			if(worm_patch_addr != 0) {
	#ifdef DEBUG_MODE
				std::cout << "Worm patch at " << std::setbase(16) << worm_patch_addr << std::endl;
	#endif

				orig_worm = new BYTE[sizeof(worm_patch)];
				spel->read_mem(worm_patch_addr, orig_worm, sizeof(worm_patch));
			}
			else {
				is_valid = false;
			}
		}
		
		{
			olmec_patch_addr = spel->find_mem(olmec_patch_find, olmec_patch_mask)-0x96;
			if(olmec_patch_addr != 0) {
	#ifdef DEBUG_MODE
				std::cout << "Olmec patch at " << std::setbase(16) << olmec_patch_addr << std::endl;
	#endif

				orig_olmec = new BYTE[sizeof(olmec_patch)];
				spel->read_mem(olmec_patch_addr, orig_olmec, sizeof(olmec_patch));
			}
			else {
				is_valid = false;
			}
		}

		{
			sparkles_patch_addr = spel->find_mem(sparkles_find, sparkles_mask)-0x13;
			if(sparkles_patch_addr != 0) {
#ifdef DEBUG_MODE
				std::cout << "Sparkles patch at " << std::setbase(16) << sparkles_patch_addr << std::endl;
#endif

				orig_sparkles = new BYTE[sizeof(sparkles_patch)];
				spel->read_mem(sparkles_patch_addr, orig_sparkles, sizeof(sparkles_patch));
			}
			else {
				is_valid = false;
			}
		}
	}

private:
	virtual bool _undo() {
		spel->write_mem(waterbubbles_patch_addr, orig_waterbubbles, sizeof(waterbubbles_patch));
		spel->write_mem(worm_patch_addr, orig_worm, sizeof(worm_patch));
		spel->write_mem(olmec_patch_addr, orig_olmec, sizeof(olmec_patch));
		spel->write_mem(sparkles_patch_addr, orig_sparkles, sizeof(sparkles_patch));
		return true;
	}

	virtual bool _perform() {
		spel->write_mem(waterbubbles_patch_addr, waterbubbles_patch, sizeof(waterbubbles_patch));
		spel->write_mem(worm_patch_addr, worm_patch, sizeof(worm_patch));
		spel->write_mem(olmec_patch_addr, olmec_patch, sizeof(olmec_patch));
		spel->write_mem(sparkles_patch_addr, sparkles_patch, sizeof(sparkles_patch));

		spel->write_mem(entity_overflow_addr, overflow_patch, sizeof(overflow_patch));
		spel->write_mem(entity_overflow2_addr, overflow2_patch, sizeof(overflow2_patch));
		return true;
	}

	virtual bool valid() {
		return is_valid;
	}
};