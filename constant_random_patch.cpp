#include "constant_random_patch.h"

#define CRP_EXEC_ALLOC 128
#define CRP_RANDOM_REPL_SIZE 5

/*
// +0x0 + 2  4= current_game_ptr
// +0x6 + 2  1= state offset
// +0xE + 2  4= current level offset
BYTE constant_random_oper[] = {
	0x8B,0x15,0xB4,0x84,0x4C,0x00,
	0x8B,0x42,0x58,
	0x83,0xF8,0x00,
	0x75,0x20,
	0x8B,0x82,0xD4,0x05,0x44,0x00,
	0x83,0xF8,0x11,
	0x74,0x05,
	0x83,0xF8,0x15,
	0x75,0x08,
	0xB8,0x21,0x20,0x01,0x00, //boss random val
	0x31,0xD2,
	0xC3,
	0xB8,0x00,0x01,0x00,0x00, //normal random val
	0x31,0xD2,
	0xC3,
	0x31,0xD2
};
*/

BYTE constant_random_oper[] = {0x8B,0x15,0xB4,0x84,0xAD,0x00,0x8B,0x42,0x58,0x83,0xF8,0x00,0x75,0x0B,0xA1,0x00,0x00,0x9B,0x0A,0x6B,0xC0,0x41,0x31,0xD2,0xC3};


ConstantRandomPatch::~ConstantRandomPatch() {
	if(orig_random) {
		delete[] orig_random;
		orig_random = nullptr;
	}

	if(routine_alloc) {
		spel->free(routine_alloc);
		routine_alloc = 0;
	}
}


//CRP is no longer required, this is just stub at this point.
ConstantRandomPatch::ConstantRandomPatch(std::shared_ptr<DerandomizePatch> dp, std::shared_ptr<GameHooks> gh, std::shared_ptr<Seeder> seeder) 
	: Patch(dp->spel), 
	dp(dp), 
	gh(gh), 
	seeder(seeder), 
	orig_random(nullptr), 
	routine_alloc(0), 
	is_valid(true)
{
	//allocate space for routine
	routine_alloc = spel->allocate(CRP_EXEC_ALLOC, true);
	if(!routine_alloc) {
		DBG_EXPR(std::cout << "[ConstantRandomPatch] Failed to allocate memory for exec routine" << std::endl);
		is_valid = false;
		return;
	}
	DBG_EXPR(std::cout << "[ConstantRandomPatch] Allocated routine space at " << routine_alloc << std::endl);

	//store + save random fn
	random_fn = dp->random_fn_address();
	orig_random = new BYTE[CRP_RANDOM_REPL_SIZE];
	spel->read_mem(random_fn, orig_random, CRP_RANDOM_REPL_SIZE);
}

bool ConstantRandomPatch::_perform() {
	spel->write_mem(routine_alloc, constant_random_oper, sizeof(constant_random_oper));

	Address current_game_ptr = dp->game_ptr();
	unsigned level_offset = dp->current_level_offset();
	BYTE state_offset = (BYTE)gh->game_state_offs();
	Address force_addr = dp->arb_alloc_force_address();

	//spel->write_mem(routine_alloc + 0x0 + 2, &current_game_ptr, sizeof(Address));
	//spel->write_mem(routine_alloc + 0x6 + 2, &state_offset, sizeof(BYTE));
	//spel->write_mem(routine_alloc + 0xE + 2, &level_offset, sizeof(unsigned));

	spel->write_mem(routine_alloc + 0x0 + 2, &current_game_ptr, sizeof(Address));
	spel->write_mem(routine_alloc + 0x6 + 2, &state_offset, sizeof(BYTE));
	spel->write_mem(routine_alloc + 0xE + 1, &force_addr, sizeof(Address));

	spel->jmp_build(random_fn, CRP_RANDOM_REPL_SIZE, routine_alloc, sizeof(constant_random_oper));
	return true;
}

bool ConstantRandomPatch::_undo() {
	spel->write_mem(random_fn, orig_random, CRP_RANDOM_REPL_SIZE);
	return true;
}

bool ConstantRandomPatch::valid() {
	return is_valid;
}