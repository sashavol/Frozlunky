#pragma once

#include "patches.h"

#define RAND_ARR_SIZE 4992
#define FUNC_RAND_ARRAY_SIZE 0xD0
#define FUNC_MERSENNE_SIZE 0x14B
#define FUNC_SEED_GEN_SIZE 0x40

class DerandomizePatch : public Patch {
private:
	BYTE* new_mersenne_code;
	BYTE* new_rand_arr_code;
	BYTE* new_seed_gen_code;

	BYTE* original_mersenne_code;
	BYTE* original_rand_arr_code;
	BYTE* original_seed_gen_code;

	BYTE* original_cont_loc1;
	BYTE* original_cont_loc2;

	Address mersenne_address;
	Address rand_arr_address;
	Address seed_gen_address;
	Address rand_fn_addr;

	Address g_RandomBytes;
	Address g_RandomIdx;
	Address g_PtrCurrentGame;
	Address g_LevelOffset;

	Address seed_cont_loc1;
	Address seed_cont_loc2;

	//used by crp / irp, otherwise have no use
	Address alloc_counter_address;
	Address alloc_force_address;
	
	bool is_valid;

	//block move
	DerandomizePatch(DerandomizePatch& oth) : Patch(oth.spel) {}
	DerandomizePatch(DerandomizePatch&& oth) : Patch(oth.spel) {}

public:
	~DerandomizePatch();
	DerandomizePatch(std::shared_ptr<Spelunky> spel);
	int current_level();
	Address current_level_offset();
	Address game_ptr();
	Address random_bytes_address();
	Address random_idx_address();
	Address random_fn_address();

	//used by crp / irp, otherwise no use.
	Address arb_alloc_counter_address();
	Address arb_alloc_force_address();

private:
	virtual bool _undo() override;
	virtual bool _perform() override;

public:
	virtual bool valid() override;
};