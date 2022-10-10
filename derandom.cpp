#include "derandom.h"

/**
	//We'll have to verify these hooks still work in the next version! (Depending on if g_RandomIdx and g_RandomBytes change)

	Globals to modify:
	g_RandomInts[0x4992 / sizeof(int)]	<Spelunky.exe+138634>
	g_RandomIdx <Spelunky.exe+138630>
	g_CurrentGame* <Spelunky.exe+...>

	Functions to Patch:

	MersenneTwister   	<Spelunky.exe+7DB0> 
	{
		0: ret;
	}


	GenerateRandArray 	<Spelunky.exe+7CD0>
	{
		0: mov [Spelunky.exe+138630], 0;
		1: ret;
	}

	InitializeSeedGenerator <Spelunky.exe+16B30>
	{
		0: mov [Spelunky.exe+138630], 0;
		1: ret;
	}
*/

std::string rand_array_mask = ".....x.x...xxxxx.x.x";
BYTE rand_array_search[] = {0xCC,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0x33,0xCC,0xCC,0xCC,0xFF,0xFF,0xFF,0x7F,0x33,0xCC,0x8B,0xCC,0x80};


std::string mersenne_mask = ".x.....x.x..xxxxx.....x.....x";
BYTE mersenne_search[] = {0xCC,0xB8,0xCC,0xCC,0xCC,0xCC,0xCC,0xEB,0xCC,0x8D,0xCC,0xCC,0x00,0x00,0x00,0x00,0x8B,0xCC,
	0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0x33};


std::string seed_gen_mask = "......xxxx......x.x..x.x.....x";
BYTE seed_gen_search[] = {0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x01,0x00,0x00,0x00,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xC1,0xCC,0xCC,0x33,0xCC,0x69,0xCC,0xCC,0xCC,0xCC,0xCC,0x8D};


std::string g_LevelOffsetContainer_mask = ".xxxxx.....x.....x.x";
BYTE g_LevelOffsetContainer_locator[] = {
	                                    // Spelunky.exe+69C10
	0xB8, 0x01, 0x00, 0x00, 0x00,       // B8 01000000    - mov eax,00000001
	0x01, 0xCC, 0xAA, 0xAA, 0xAA, 0xAA, // 01 87 D4054400 - add [edi+Spelunky.exe+305D4],eax
	0x38, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // 38 8F 00064400 - cmp [edi+Spelunky.exe+30600],cl
	0x74, 0xCC,                         // 74 0E          - je Spelunky.exe+69C31
	0x88                                // 88 8F FF054400 - mov [edi+Spelunky.exe+305FF],cl
};

std::string g_PtrCurrentGameContainer_mask = "x..x....x....x.....x";
BYTE g_PtrCurrentGameContainer_locator[] = {
	                                    // Spelunky.exe+00069983
	0x8D, 0xCC, 0xCC,                   // 8D 70 88       - lea esi,[eax-78]
	0xBF, 0xCC, 0xCC, 0xCC, 0xCC,       // BF BE010000    - mov edi,000001BE
	0xE8, 0xCC, 0xCC, 0xCC, 0xCC,       // E8 A0DE0300    - call Spelunky.exe+A7830
	0x8B, 0xCC, 0xAA, 0xAA, 0xAA, 0xAA, // 8B 35 B4845400 - mov esi,[Spelunky.exe+1384B4]
	0x80                                // 80 7E 70 00    - cmp byte ptr [esi+70],00
}; 

//instance assembly
/*
	push edi
	push ecx
	push eax
	mov edi,[014C84B4]
	mov eax, [edi+4405D4]
	
	mov [014C8630], eax
	pop eax
	pop ecx
	pop edi
	ret
*/
																//g_PtrCurrentGame			
BYTE reset_idx_patch_template[] = {0x57, 0x50, 0x51, 0x8B, 0x3D, 0x00, 0x00, 0x00, 0x00, 
					//g_LevelOffset	
	0x8B, 0x8F, 0x00, 0x00, 0x00, 0x00, 
	0xB8, 0x04, 0x00, 0x00, 0x00, 0xF7, 0xE1, 0x89, 0x05,
		//g_RandomIdx
	0x00, 0x00, 0x00, 0x00, 0x59, 0x58, 0x5F, 0xC3};


std::string seed_cont_loc1_mask = "x.....x.x..x....x";
BYTE seed_cont_loc1_find[] = {
	// 38 9e 28 06 44 00        CMP  byte ptr [ESI + 0x440628]=>LAB_00040627+1,BL
	0x38,0xCC,0xCC,0xCC,0xCC,0xCC,           
	// 74 08                    JZ   LAB_00032c54
	0x74,0xCC,                               
	// 8b 47 2c                 MOV  EAX,dword ptr [EDI + 0x2c]
	0x8B,0xCC,0xCC,                          
	// e8 1c e5 fd ff           CALL FUN_00011170 
	0xE8,0xCC,0xCC,0xCC,0xCC,                
	// 8b 87 44 01 00 00        MOV  EAX,dword ptr [EDI + 0x144]
	0x8B                                    
};
BYTE seed_cont_loc1_patch[] = {
	0x8B, 0x47, 0x44,                   // 8b 47 44           mov    eax,DWORD PTR [edi+0x44]
	0x25, 0xFF, 0x00, 0x00, 0x00,       // 25 ff 00 00 00     and    eax,0xff
	0x89, 0x05, 0xAA, 0xAA, 0xAA, 0xAA, // 89 05 aa aa aa aa  mov    DWORD PTR ds:0xaaaaaaaa,eax
	0x90,                               // 90                 nop
	0x90                                // 90                 nop
}; //patches {seed_cont_loc1_find} + 0x8, par1 at 8


std::string seed_cont_loc2_mask = "x..x.....x.....x.x..x....x";
BYTE seed_cont_loc2_find[] = {
	// 83 f8 64               CMP        EAX,0x64
	0x83, 0xCC, 0xCC,
	// 0f 85 80 02 00 00      JNZ        LAB_0003340c
	0x0F, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
	// 38 9e 28 06 44 00      CMP        byte ptr [ESI + 0x440628]=>LAB_00040627+1,BL
	0x38, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
	// 74 08                  JZ         LAB_0003319c
	0x74,0xCC,
	// 8b 47 2c               MOV        EAX,dword ptr [EDI + 0x2c]
	0x8B, 0xCC, 0xCC,
	// e8 d4 df fd ff         CALL       FUN_00011170
	0xE8, 0xCC, 0xCC, 0xCC, 0xCC,
	// 83 bf 44 01 00 00 01   CMP        dword ptr [EDI + 0x144],0x1
	0x83
};
BYTE seed_cont_loc2_patch[] = {
	0x8B, 0x47, 0x44,                   // 8b 47 44           mov    eax,DWORD PTR [edi+0x44]
	0x25, 0xFF, 0x00, 0x00, 0x00,       // 25 ff 00 00 00     and    eax,0xff
	0x89, 0x05, 0xAA, 0xAA, 0xAA, 0xAA, // 89 05 aa aa aa aa  mov    DWORD PTR ds:0xaaaaaaaa,eax
	0x90,                               // 90                 nop
	0x90                                // 90                 nop
}; //patches {seed_cont_loc2_find} + 0x9, par1 at 8


//+2
BYTE random_fn_find[] = {0xCC,0xCC,0xA1,0xAA,0xAA,0xAA,0xAA,0x3D,0xAA,0xAA,0xAA,0xAA,0x75,0xAA,0xE8,0xAA,0xAA,0xAA,0xAA,0xEB,0xAA,0x3D};
std::string random_fn_mask = "xxx....x....x.x....x.x";

DerandomizePatch::~DerandomizePatch() {
#define DEL_NOT_NULL(PTR) if(PTR != nullptr) {delete[] PTR;}
	DEL_NOT_NULL(new_mersenne_code);
	DEL_NOT_NULL(new_rand_arr_code);
	DEL_NOT_NULL(new_seed_gen_code);
	DEL_NOT_NULL(original_cont_loc1);
	DEL_NOT_NULL(original_cont_loc2);

	DEL_NOT_NULL(original_mersenne_code);
	DEL_NOT_NULL(original_rand_arr_code);
	DEL_NOT_NULL(original_seed_gen_code);
#undef DEL_NOT_NULL
}

DerandomizePatch::DerandomizePatch(std::shared_ptr<Spelunky> spel) : Patch(spel), 
	original_mersenne_code(nullptr), 
	original_rand_arr_code(nullptr), 
	original_seed_gen_code(nullptr),
	original_cont_loc1(nullptr),
	original_cont_loc2(nullptr),
	new_mersenne_code(nullptr),
	new_rand_arr_code(nullptr),
	new_seed_gen_code(nullptr),
	alloc_counter_address(0),
	alloc_force_address(0),
	is_valid(true)
{
	new_mersenne_code = new BYTE[FUNC_MERSENNE_SIZE];
	new_rand_arr_code = new BYTE[FUNC_RAND_ARRAY_SIZE];
	new_seed_gen_code = new BYTE[FUNC_SEED_GEN_SIZE];

	original_mersenne_code = new BYTE[FUNC_MERSENNE_SIZE];
	original_rand_arr_code = new BYTE[FUNC_RAND_ARRAY_SIZE];
	original_seed_gen_code = new BYTE[FUNC_SEED_GEN_SIZE];
	original_cont_loc1 = new BYTE[sizeof(seed_cont_loc1_patch)];
	original_cont_loc2 = new BYTE[sizeof(seed_cont_loc2_patch)];

	BYTE reset_idx_patch[sizeof(reset_idx_patch_template)];
	std::memcpy(reset_idx_patch, reset_idx_patch_template, sizeof(reset_idx_patch_template));

	//reset idx patch
	{
		Address g_PtrCurrentGameContainer = spel->find_mem(g_PtrCurrentGameContainer_locator, g_PtrCurrentGameContainer_mask);
		if(g_PtrCurrentGameContainer == 0) {
			is_valid = false;
			throw std::runtime_error("Could not find game information struct in memory.");
		}

		spel->read_mem(g_PtrCurrentGameContainer+15, &g_PtrCurrentGame, sizeof(Address));

		Address g_LevelOffsetContainer = spel->find_mem(g_LevelOffsetContainer_locator, g_LevelOffsetContainer_mask);
		if(g_LevelOffsetContainer == 0) {
			is_valid = false;
			throw std::runtime_error("Could not find game struct level number offset in memory.");
		}

		spel->read_mem(g_LevelOffsetContainer+7, &g_LevelOffset, sizeof(Address));

		DBG_EXPR(
			std::cout << "current game container: " << std::setbase(16) << g_PtrCurrentGameContainer << std::endl;
			std::cout << "current game: " << std::setbase(16) << g_PtrCurrentGame << std::endl;
			std::cout << "Level offset container: " << std::setbase(16) << g_LevelOffsetContainer << std::endl;
			std::cout << "Level offset: " << std::setbase(16) << g_LevelOffset << std::endl;
		);

		std::memcpy(reset_idx_patch+5, &g_PtrCurrentGame, sizeof(Address));
		std::memcpy(reset_idx_patch+11, &g_LevelOffset, sizeof(Address));
	}

	//InitializeSeedGenerator identifier
	{
		seed_gen_address = spel->find_mem(seed_gen_search, seed_gen_mask);
		if(seed_gen_address == 0) {
			is_valid = false;
			throw std::runtime_error("Could not find offset for InitializeSeedGenerator.");
		}
		spel->read_mem(seed_gen_address, original_seed_gen_code, FUNC_SEED_GEN_SIZE);
		std::memcpy(new_seed_gen_code, original_seed_gen_code, FUNC_SEED_GEN_SIZE);

		std::memcpy(&g_RandomBytes, original_seed_gen_code+0x1, sizeof(Address));
		std::memcpy(&g_RandomIdx, original_seed_gen_code+0x30, sizeof(Address));

		DBG_EXPR(
			std::cout << "Found seed_gen_address at " << seed_gen_address << std::endl;
			std::cout << "current random idx: " << std::setbase(16) << g_RandomIdx << std::endl;
			std::cout << "random byte array: " << std::setbase(16) << g_RandomBytes << std::endl;
		);

		std::memcpy(reset_idx_patch+24, &g_RandomIdx, sizeof(Address));

		std::memcpy(new_seed_gen_code, reset_idx_patch, sizeof(reset_idx_patch_template));

		unsigned val = 0;
		spel->write_mem(g_RandomIdx, &val, sizeof(unsigned));
	}

	//MersenneTwister identifier
	{
		mersenne_address = spel->find_mem(mersenne_search, mersenne_mask);
		if(mersenne_address == 0) {
			is_valid = false;
			throw std::runtime_error("Could not find offset for MersenneTwister.");
		}
		spel->read_mem(mersenne_address, original_mersenne_code, FUNC_MERSENNE_SIZE);
		std::memcpy(new_mersenne_code, original_mersenne_code, FUNC_MERSENNE_SIZE);
		std::memcpy(new_mersenne_code, reset_idx_patch, sizeof(reset_idx_patch_template));

#ifdef DEBUG_MODE
		std::cout << "Found mersenne_address at " << mersenne_address << std::endl;
#endif
	}

	//GenerateRandArray identifier
	{
		rand_arr_address = spel->find_mem(rand_array_search, rand_array_mask);
		if(rand_arr_address == 0) {
			is_valid = false;
			throw std::runtime_error("Could not find offset for GenerateRandArray.");
		}
		spel->read_mem(rand_arr_address, original_rand_arr_code, FUNC_RAND_ARRAY_SIZE);
		std::memcpy(new_rand_arr_code, original_rand_arr_code, FUNC_RAND_ARRAY_SIZE);

		std::memcpy(new_rand_arr_code, reset_idx_patch, sizeof(reset_idx_patch_template));

#ifdef DEBUG_MODE
		std::cout << "Found rand_array_mask at " << rand_arr_address << std::endl;
#endif
	}



	//Container Seeding
	{
		Address cont1 = spel->find_mem(seed_cont_loc1_find, seed_cont_loc1_mask);
		Address cont2 = spel->find_mem(seed_cont_loc2_find, seed_cont_loc2_mask);

		if(cont1 == 0x0 || cont2 == 0x0) {
			is_valid = false;
			throw std::runtime_error("Could not find offset for seeding containers.");
		}

		//offsets
		cont1 += 0x0;
		cont2 += 0x9;

		spel->read_mem(cont1, original_cont_loc1, sizeof(seed_cont_loc1_patch));
		spel->read_mem(cont2, original_cont_loc2, sizeof(seed_cont_loc2_patch));

		seed_cont_loc1 = cont1;
		seed_cont_loc2 = cont2;

#ifdef DEBUG_MODE
		std::cout << "Found container seeding offset 1 at " << std::setbase(16) << seed_cont_loc1 << std::endl;
		std::cout << "Found container seeding offset 2 at " << std::setbase(16) << seed_cont_loc2 << std::endl;
#endif
	}


	//Random fn finder
	{
		rand_fn_addr = spel->find_mem(random_fn_find, random_fn_mask);
		if(!rand_fn_addr) {
			DBG_EXPR(std::cout << "[DerandomizePatch] Failed to find Random()" << std::endl);
			is_valid = false;
			throw std::runtime_error("Failed to find Random()");
		}
		rand_fn_addr += 2;
		DBG_EXPR(std::cout << "[DerandomizePatch] Found Random() at " << rand_fn_addr << std::endl);
	}


	//allocs
	{
		int zero = 0;
		alloc_counter_address = spel->allocate(sizeof(int));
		alloc_force_address = spel->allocate(sizeof(int));
		spel->write_mem(alloc_counter_address, &zero, sizeof(int));
		spel->write_mem(alloc_force_address, &zero, sizeof(int));

		if(!alloc_counter_address || !alloc_force_address) {
			DBG_EXPR(std::cout << "[DerandomizePatch] Failed to allocate alloc_counter_address | alloc_force_address" << std::endl);
			is_valid = false;
			throw std::runtime_error("Failed to alloc");
		}
		DBG_EXPR(std::cout << "[DerandomizePatch] alloc_counter_address == " << alloc_counter_address << std::endl);
		DBG_EXPR(std::cout << "[DerandomizePatch] alloc_force_address == " << alloc_force_address << std::endl);
	}
}

int DerandomizePatch::current_level() {
	if(is_valid) {
		Address current_game;
		spel->read_mem(g_PtrCurrentGame, &current_game, sizeof(Address));

		int level;
		spel->read_mem(current_game + g_LevelOffset, &level, sizeof(int));

		return level;
	}
	else {
		return -1;
	}
}

Address DerandomizePatch::current_level_offset() {
	return g_LevelOffset;
}

Address DerandomizePatch::game_ptr() {
	return g_PtrCurrentGame;
}

Address DerandomizePatch::random_bytes_address() {
	return g_RandomBytes;
}

Address DerandomizePatch::random_idx_address() {
	return g_RandomIdx;
}

Address DerandomizePatch::random_fn_address() {
	return rand_fn_addr;
}

bool DerandomizePatch::_undo() {
	spel->write_mem(rand_arr_address, original_rand_arr_code, FUNC_RAND_ARRAY_SIZE); 
	spel->write_mem(mersenne_address, original_mersenne_code, FUNC_MERSENNE_SIZE); 
	spel->write_mem(seed_gen_address, original_seed_gen_code, FUNC_SEED_GEN_SIZE); 
	spel->write_mem(seed_cont_loc1, original_cont_loc1, sizeof(seed_cont_loc1_patch));
	spel->write_mem(seed_cont_loc2, original_cont_loc2, sizeof(seed_cont_loc2_patch));
	return true;
}

bool DerandomizePatch::_perform() {
	spel->write_mem(rand_arr_address, new_rand_arr_code, FUNC_RAND_ARRAY_SIZE); 
	spel->write_mem(mersenne_address, new_mersenne_code, FUNC_MERSENNE_SIZE); 
	spel->write_mem(seed_gen_address, new_seed_gen_code, FUNC_SEED_GEN_SIZE); 

	spel->write_mem(seed_cont_loc1, seed_cont_loc1_patch, sizeof(seed_cont_loc1_patch));
	spel->write_mem(seed_cont_loc1+10, &g_RandomIdx, sizeof(Address));

	spel->write_mem(seed_cont_loc2, seed_cont_loc2_patch, sizeof(seed_cont_loc2_patch));
	spel->write_mem(seed_cont_loc2+10, &g_RandomIdx, sizeof(Address));

	return true;
}

bool DerandomizePatch::valid() {
	return is_valid;
}

Address DerandomizePatch::arb_alloc_counter_address() {
	return alloc_counter_address;
}

Address DerandomizePatch::arb_alloc_force_address() {
	return alloc_force_address;
}