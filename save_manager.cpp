#include "save_manager.h"
#include "savefile.h"
#include "rc_io.h"
#include "force_controller_attrs.h"

//+0
BYTE whitescreen_find[] = {0xDB,0xCC,0xCC,0xCC,0xCC,0xCC,0xD9,0xCC,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xDC,0xCC,0xD9,0xCC,0xD9};
std::string whitescreen_mask = "x.....x.....x..x.x.x";
BYTE whitescreen_override[] = {0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90};

//+0
BYTE options_find[] = {0x83,0xCC,0xCC,0x8D,0xCC,0xCC,0x8D,0xCC,0xCC,0xCC,0x8B,0xCC,0xBA,0xCC,0xCC,0xCC,0xCC,0x8B};
std::string options_mask = "x..x..x...x.x....x";
BYTE options_override[] = {0x5F,0x5E,0x5B,0xC3};

SaveManager::~SaveManager() {
	if(whitescreen_orig) {
		delete[] whitescreen_orig;
		whitescreen_orig = nullptr;
	}

	if(options_orig) {
		delete[] options_orig;
		options_orig = nullptr;
	}
}

SaveManager::SaveManager(std::shared_ptr<RemoteCallPatch> rcp, std::shared_ptr<GameHooks> gh, std::shared_ptr<DerandomizePatch> dp) : 
	rcp(rcp),
	gh(gh),
	dp(dp),
	options_addr(0),
	is_valid(true),
	options_orig(nullptr),
	whitescreen_addr(0),
	whitescreen_orig(nullptr)
{
	std::shared_ptr<Spelunky> spel = rcp->spel;

	{
		whitescreen_addr = spel->find_mem(whitescreen_find, whitescreen_mask);
		if(!whitescreen_addr) {
			DBG_EXPR(std::cout << "[SaveManager] Error finding white screen patch address." << std::endl);
			is_valid = false;
			return;
		}
		DBG_EXPR(std::cout << "[SaveManager] whitescreen_addr = " << whitescreen_addr << std::endl);

		whitescreen_orig = new BYTE[sizeof(whitescreen_override)];
		spel->read_mem(whitescreen_addr, whitescreen_orig, sizeof(whitescreen_override));
	}

	{
		options_addr = spel->find_mem(options_find, options_mask);
		if(!options_addr) {
			DBG_EXPR(std::cout << "[SaveManager] Error finding options save " << std::endl);
			is_valid = false;
			return;
		}
		DBG_EXPR(std::cout << "[SaveManager] options_addr = " << options_addr << std::endl);

		options_orig = new BYTE[sizeof(options_override)];
		spel->read_mem(options_addr, options_orig, sizeof(options_override));
	}
}

void SaveManager::load_netplay_save(std::function<void(bool)> cb) {
	if(!is_valid) {
		if(cb) { cb(false); }
		return;
	}

	DBG_EXPR(std::cout << "[SaveManager] Writing new savefile. " << std::endl);
	if(!write_savefile()) {
		if(cb) { cb(false); }
		return;
	}

	DBG_EXPR(std::cout << "[SaveManager] Loading save." << std::endl);
	patch();
	if(!rcp->enqueue_call(std::make_shared<RCLoad>(dp)->make(), [=]() 
		{
			DBG_EXPR(std::cout << "[SaveManager] Save load successful." << std::endl);
			unpatch();
			if(cb) { cb(true); }
		}))
	{
		DBG_EXPR(std::cout << "[SaveManager] Failed to enqueue load call." << std::endl);
		if(cb) { cb(false); }
	}
}

bool SaveManager::valid() {
	return is_valid;
}

bool SaveManager::write_savefile() {
	FILE* out = NULL;
	fopen_s(&out, (rcp->spel->base_directory() + "Data\\frzlunky_save.sav").c_str(), "wb+");
	if(!out) {
		DBG_EXPR(std::cout << "[SaveManager] Failed to open " << (rcp->spel->base_directory() + "Data\\frzlunky_save.sav") << " with wb+ " << std::endl);
		return false;
	}

	if(fwrite((const void*)Savefile::File, sizeof(unsigned char), sizeof(Savefile::File), out) != sizeof(Savefile::File)) {
		DBG_EXPR(std::cout << "[SaveManager] Unknown error prevented from fully writing to frozlunky_save." << std::endl);
		fclose(out);
		return false;
	}

	fclose(out);

	return true;
}

void SaveManager::patch() {
	rcp->spel->write_mem(whitescreen_addr, whitescreen_override, sizeof(whitescreen_override));
	rcp->spel->write_mem(options_addr, options_override, sizeof(options_override));
}

void SaveManager::unpatch() {
	rcp->spel->write_mem(whitescreen_addr, whitescreen_orig, sizeof(whitescreen_override));
	rcp->spel->write_mem(options_addr, options_override, sizeof(options_override));
}