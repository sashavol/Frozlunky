#pragma once

#include "patches.h"
#include "derandom.h"
#include <iostream>

										 //dp->random_idx_address()
BYTE rand_func_find[] = {0x81, 0xE2, 0xAD, 0x58, 0x3A, 0xFF, 0xC1, 0xE2, 0x07, 0x33, 0xCA, 0x8B, 0xC1, 0x25, 0x8C, 0xDF, 0xFF, 0xFF, 0xC1, 0xE0, 0x0F, 0x33, 0xC8, 0x8B, 0xC1, 0xC1, 0xE8, 0x12, 0x33, 0xC1, 0xC3};
std::string rand_func_mask = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
						     

// A  <+4 >  =  g_RandomIdx 
// B  <+9 >  =  g_CurrentGamePtr
// C  <+14>  =  g_CurrentLevelOffs
// D  <+27>  =  g_RandomBytes
// E  <+33>  =  diff srand*
// F  <+41>  =  diff rand*
// AB <+49>  =  diff Random

BYTE secrand_template[] = {0x51, 0x53, 0x8B, 0x1D, 0xAA, 0xAA, 0xAA, 0xAA, 0xA1, 0xBB, 0xBB, 0xBB, 0xBB, 0x05, 0xCC, 0xCC, 0xCC, 0xCC, 0x8B, 0x08, 0x8B, 0xC1, 0x01, 0xD8, 0x03, 0x04, 0x9D, 0xDD, 0xDD, 0xDD, 0xDD, 0x50, 0xE8, 0xEE, 0xEE, 0xEE, 0xEE, 0x83, 0xC4, 0x04, 0xE8, 0xFF, 0xFF, 0xFF, 0xFF, 0x8B, 0xC1, 0x50, 0xE8, 0xAB, 0xAB, 0xAB, 0xAB, 0x58, 0x5B, 0x59, 0xC3};

BYTE secrand_empty_find[] = {0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC};
std::string secrand_empty_mask = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

class SecondaryRandomPatch : public Patch {
private:
	std::shared_ptr<DerandomizePatch> dp;
	bool is_valid;
	Address secrand;
	Address random_func_addr;
	BYTE* secrand_func;

	virtual bool _perform() override {
		spel->write_mem(secrand, secrand_func, sizeof(secrand_template), true); 
		return true;
	}

	virtual bool _undo() override {
		BYTE padding[sizeof(secrand_template)];
		for(size_t i = 0; i < sizeof(padding); i++) {
			padding[i] = 0xCC;
		}
		spel->write_mem(secrand, padding, sizeof(secrand_template));
		return true;
	}

public:
	~SecondaryRandomPatch() {
		delete[] secrand_func;
	}

	SecondaryRandomPatch(std::shared_ptr<DerandomizePatch> dp) : Patch(dp->spel), 
		dp(dp),
		is_valid(true) 
	{
		secrand = spel->find_exec_mem(secrand_empty_find, secrand_empty_mask);
		if(secrand == 0x0) {
			is_valid = false;
			return;
		}


		random_func_addr = spel->find_mem(rand_func_find, rand_func_mask);
		if(random_func_addr == 0x0) {
			is_valid = false;
			return;
		}
		random_func_addr -= 0x3A;


		secrand_func = new BYTE[sizeof(secrand_template)];
		std::memcpy(secrand_func, secrand_template, sizeof(secrand_template));

		Address g_CurrentGamePtr = dp->game_ptr();
		Address current_level_offs = dp->current_level_offset();
		Address g_RandomBytes = dp->random_bytes_address();
		Address g_RandomIdx = dp->random_idx_address();

		std::memcpy(secrand_func+4, &g_RandomIdx, sizeof(Address));
		std::memcpy(secrand_func+9, &g_CurrentGamePtr, sizeof(Address));
		std::memcpy(secrand_func+14, &current_level_offs, sizeof(Address));
		std::memcpy(secrand_func+27, &g_RandomBytes, sizeof(Address));

		Address srand = spel->get_libc_func("srand");
		Address rand = spel->get_libc_func("rand");

		Address diff_srand = srand - (secrand+32) - 5;
		Address diff_rand = rand - (secrand+40) - 5;
		Address diff_random = random_func_addr - (secrand+48) - 5;

		std::memcpy(secrand_func+33, &diff_srand, sizeof(Address));
		std::memcpy(secrand_func+41, &diff_rand, sizeof(Address));
		std::memcpy(secrand_func+49, &diff_random, sizeof(Address));

#ifdef DEBUG_MODE
		std::cout << "Creating secrand at " << std::setbase(16) << secrand << std::endl;
		std::cout << "Found Random() @ " << std::setbase(16) << random_func_addr << std::endl;
		std::cout << "srand @ " << std::setbase(16) << srand << " (diff == " << diff_srand << ")" << std::endl;
		std::cout << "rand @ " << std::setbase(16) << rand << " (diff == " << diff_rand << ")" << std::endl;
#endif
	}

	virtual bool valid() override {
		return is_valid;
	}

	Address secrand_addr() {
		return secrand;
	}
};