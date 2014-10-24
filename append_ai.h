#pragma once

#include "patches.h"
#include <vector>
#include <iostream>
#include <map>

#define ENTITY_FROG 0x3EF
#define ENTITY_ORANGEFROG 0x3FD
#define ENTITY_BIG_FROG 0x40E
#define ENTITY_COBRA 0x40C
#define ENTITY_SNAKE 0x3E9
#define ENTITY_ANUBIS22 0x409
#define ENTITY_ANUBIS2 0x41E
#define ENTITY_ANUBIS 0x3F8
#define ENTITY_SPIDER3 0x3EA
#define ENTITY_SPIDER2 0x3FA
#define ENTITY_SPIDER 0x40D
#define ENTITY_BAT 0x3EB
#define ENTITY_DAMSEL 0x3ED
#define ENTITY_UFO 0x3F2
#define ENTITY_PIRANHA 0x3F5
#define ENTITY_VOMITFLIES 0x3F6
#define ENTITY_YETI 0x401
#define ENTITY_CHIMP 0x3F7
#define ENTITY_GHOST 0x3F9
#define ENTITY_ALIEN 0x402
#define ENTITY_IMP 0x406
#define ENTITY_SCORPION 0x405
#define ENTITY_BEE 0x408
#define ENTITY_BEE2 0x40A
#define ENTITY_MAMMOTH 0x40F
#define ENTITY_TANK 0x410
#define ENTITY_SNAIL 0x413
#define ENTITY_ALIEN_QUEEN 0x418
#define ENTITY_TURRET 0x422
#define ENTITY_YAMA 0x420
#define ENTITY_PENGUIN 0x428
#define ENTITY_FROG_CRITTER 0x400
#define ENTITY_SHOPKEEPER 0x3EE
#define ENTITY_BACTERIA 0x40B
#define ENTITY_BLUEDEVIL 0x407
#define ENTITY_CAVEMAN 0x411
#define ENTITY_CAVEMAN2 0x415
#define ENTITY_SCORPION_FLY 0x412
#define ENTITY_HAWK_MAN 0x414
#define ENTITY_WORM_BABY 0x417
#define ENTITY_BLACK_KNIGHT 0x419
#define ENTITY_FISH_CRITTER 0x403
#define ENTITY_PIRANHA_CRITTER 0x426
#define ENTITY_VAMPIRE 0x404
#define ENTITY_SUCCUBUS 0x41B
#define ENTITY_HORSEHEAD 0x41C
#define ENTITY_OXFACE 0x41D

typedef unsigned short entity_id;

struct EntityType {
	unsigned id;
	BYTE movtype;
	Address behave_addr;

	EntityType(unsigned id, BYTE movtype, Address behave_addr) : id(id), movtype(movtype), behave_addr(behave_addr) {}
	EntityType() {}
};

class AppendAIPatch : public Patch {
private:
	std::vector<unsigned> available_ids;
	std::map<unsigned, EntityType> entity_data;

	BYTE* original_insert_mem;

	virtual bool _perform() override;
	virtual bool _undo() override;

public:
	typedef std::vector<unsigned> entity_storage;

	std::shared_ptr<Spelunky> spel;

	Address exec_space;
	Address insert_addr;
	entity_storage current_appended;
	entity_storage ignored;
	bool is_valid;

	AppendAIPatch(std::shared_ptr<Spelunky> spel);
	~AppendAIPatch();
	
	virtual bool valid() override {
		return is_valid;
	}

	entity_storage ignored_entities() {
		return this->ignored;
	}

	void set_ignored_entities(entity_storage& entities) {
		this->ignored = entities;
		if(this->is_active()) {
			this->_perform();
		}
	}

	entity_storage entities() {
		return available_ids;
	}

	bool set_appended_ai(const entity_storage& entity);
	entity_storage appended_ai();

	static std::string FriendlyName(unsigned entity);
};