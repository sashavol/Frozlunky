#pragma once

#define DBG_ADDR(arg) std::printf("%s -> 0x%x\n", #arg, arg);

#include "patches.h"
#include "debug.h"
#include <unordered_map>

const std::string mask_pHUD_patch = "xxxxxxxx";
static BYTE pHUD_patch_find[] = {0x00, 0x00, 0x48, 0x41, 0xAB, 0xAA, 0x2A, 0x3F};
static BYTE pHUD_patch_patch[] = {0x00, 0x00, 0x4A, 0x41, 0xAB, 0xAA, 0x2A, 0x3F};

const std::string mask_nHUD_patch = "xxxxxxxx";
static BYTE nHUD_patch_find[] = {0x9A, 0x99, 0xD9, 0x3F, 0x00, 0x00, 0x8E, 0x42};
static BYTE nHUD_patch_patch[] = {0x9A, 0x99, 0xF0, 0x3F, 0x00, 0x00, 0x8E, 0x42};

class WatermarkPatch : public Patch {
private:
	std::unordered_map<Address, std::pair<BYTE*, size_t>> original;
	std::unordered_map<Address, std::pair<BYTE*, size_t>> patched;
	bool is_valid;

public:
	~WatermarkPatch() {
		for(auto& orig : original) {
			 delete[] orig.second.first;
		}

		for(auto& nw : patched) {
			delete[] nw.second.first;
		}
	}

	WatermarkPatch(std::shared_ptr<Spelunky> spel) : Patch(spel), is_valid(true) {
		//pro hud patch
		{
			Address phud_patch_addr = spel->find_mem(pHUD_patch_find, mask_pHUD_patch);

			if(phud_patch_addr != 0) {
				BYTE* orig = new BYTE[sizeof(pHUD_patch_patch)];
				BYTE* nw = new BYTE[sizeof(pHUD_patch_patch)];

				spel->read_mem(phud_patch_addr, orig, sizeof(pHUD_patch_patch));
				memcpy(nw, pHUD_patch_patch, sizeof(pHUD_patch_patch));
			
				original[phud_patch_addr] = std::pair<BYTE*, size_t>(orig, sizeof(pHUD_patch_patch));
				patched[phud_patch_addr] = std::pair<BYTE*, size_t>(nw, sizeof(pHUD_patch_patch));
			}
			else {
				is_valid = false;
				return;
			}

#ifdef DEBUG_MODE
			DBG_ADDR(phud_patch_addr);
#endif
		}

		{
			Address nhud_patch_addr = spel->find_mem(nHUD_patch_find, mask_nHUD_patch);

			if(nhud_patch_addr != 0) {
				BYTE* orig = new BYTE[sizeof(nHUD_patch_patch)];
				BYTE* nw = new BYTE[sizeof(nHUD_patch_patch)];

				spel->read_mem(nhud_patch_addr, orig, sizeof(nHUD_patch_patch));
				memcpy(nw, nHUD_patch_patch, sizeof(nHUD_patch_patch));
			
				original[nhud_patch_addr] = std::pair<BYTE*, size_t>(orig, sizeof(nHUD_patch_patch));
				patched[nhud_patch_addr] = std::pair<BYTE*, size_t>(nw, sizeof(nHUD_patch_patch));
			}
			else
				is_valid = false;

#ifdef DEBUG_MODE
			DBG_ADDR(nhud_patch_addr);
#endif
		}
	}

	virtual bool valid() override {
		return is_valid;
	}

private:
	virtual bool _perform() override {
		for(auto patch : patched) {
			auto detail = patch.second;
			spel->write_mem(patch.first, detail.first, detail.second, true);
		}
		return true;
	}

	virtual bool _undo() override {
		//for(auto orig : original) {
		//	auto detail = orig.second;
		//	spel->write_mem(orig.first, detail.first, detail.second, true);
		//}
		return true;
	}
};