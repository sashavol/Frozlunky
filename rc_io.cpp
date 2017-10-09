#include "rc_io.h"

//+0
BYTE fn_load_find[] = {0x81,0xCC,0xCC,0xCC,0xCC,0xCC,0x68,0xCC,0xCC,0xCC,0xCC,0xFF,0xCC,0xCC,0xCC,0xCC,0xCC,0x68,0xCC,0xCC,0xCC,0xCC,0x8D};
std::string fn_load_mask = "x.....x....x.....x....x";

//store current game to esi
BYTE rc_load_template[] = {0x8B, 0x35, 0xCC,0xCC,0xCC,0xCC}; 

RCLoad::RCLoad(std::shared_ptr<DerandomizePatch> dp) : 
	dp(dp),
	fn_load(0x0),
	is_valid(true)
{
	std::shared_ptr<Spelunky> spel = dp->spel;

	fn_load = spel->find_mem(fn_load_find, fn_load_mask);
	if(!fn_load) {
		DBG_EXPR(std::cout << "[RC::IO] fn_load not found." << std::endl);
		is_valid = false;
		return;
	}
	DBG_EXPR(std::cout << "[RC::IO] fn_load = " << fn_load << std::endl);
}

std::shared_ptr<RemoteCallConstructor::RCData> RCLoad::make() {
	if(!is_valid) {
		return nullptr;
	}

	Address game_ptr = dp->game_ptr();

	BYTE* ctx = new BYTE[sizeof(rc_load_template)];
	std::memcpy(ctx, rc_load_template, sizeof(rc_load_template));
	std::memcpy(ctx + 2, &game_ptr, sizeof(Address));

	return std::make_shared<RCData>(fn_load, (const BYTE*)ctx, sizeof(rc_load_template));
}


//+0
BYTE fn_save_find[] = {0x81,0xCC,0xCC,0xCC,0xCC,0xCC,0x68,0xCC,0xCC,0xCC,0xCC,0xE8,0xCC,0xCC,0xCC,0xCC,0x68,0xCC,0xCC,0xCC,0xCC,0x8D};
std::string fn_save_mask = "x.....x....x....x....x";
BYTE rc_save_template[] = {0xFF, 0x35, 0xCC,0xCC,0xCC,0xCC}; //push current game to stack

RCSave::RCSave(std::shared_ptr<DerandomizePatch> dp) : 
	dp(dp),
	fn_save(0x0),
	is_valid(true)
{
	std::shared_ptr<Spelunky> spel = dp->spel;

	fn_save = spel->find_mem(fn_save_find, fn_save_mask);
	if(!fn_save) {
		DBG_EXPR(std::cout << "[RC::IO] fn_save not found" << std::endl);
		is_valid = false;
		return;
	}
	DBG_EXPR(std::cout << "[RC::IO] fn_save = " << fn_save << std::endl);
}

std::shared_ptr<RemoteCallConstructor::RCData> RCSave::make() {
	if(!is_valid) {
		return nullptr;
	}

	Address game_ptr = dp->game_ptr();

	BYTE* ctx = new BYTE[sizeof(rc_save_template)]; 
	std::memcpy(ctx, rc_save_template, sizeof(rc_save_template));
	std::memcpy(ctx + 2, &game_ptr, sizeof(Address));

	return std::make_shared<RCData>(fn_save, (const BYTE*)ctx, sizeof(rc_save_template));
}



//+0
BYTE fn_reset_find[] = {0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0x68,0xCC,0xCC,0xCC,0xCC,0x33,0xCC,0x8D,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xE8};
std::string fn_reset_mask = "....x.x....x.x.......x";
BYTE rc_reset_template[] = {0xA1, 0xCC,0xCC,0xCC,0xCC, 0x6A,0x01};

RCReset::RCReset(std::shared_ptr<DerandomizePatch> dp) : 
	dp(dp),
	fn_reset(0x0),
	is_valid(true)
{
	std::shared_ptr<Spelunky> spel = dp->spel;

	fn_reset = spel->find_mem(fn_reset_find, fn_reset_mask);
	if(!fn_reset) {
		DBG_EXPR(std::cout << "[RCReset] Failed to find fn_reset" << std::endl);
		is_valid = false;
		return;
	}
	DBG_EXPR(std::cout << "[RCReset] fn_reset = " << fn_reset << std::endl);
}

std::shared_ptr<RemoteCallConstructor::RCData> RCReset::make() {
	if(!is_valid) {
		return nullptr;
	}

	Address game_ptr = dp->game_ptr();

	BYTE* ctx = new BYTE[sizeof(rc_reset_template)]; 
	std::memcpy(ctx, rc_reset_template, sizeof(rc_reset_template));
	std::memcpy(ctx + 1, &game_ptr, sizeof(Address));

	return std::make_shared<RCData>(fn_reset, (const BYTE*)ctx, sizeof(rc_reset_template));
}