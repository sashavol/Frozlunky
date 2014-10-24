#pragma once

#include "patches.h"
#include "debug.h"
#include <vector>

std::string offline_patch_mask = "xxxxxx";
BYTE offline_patch_each_template[] = {0xFF, 0x15, 0x00, 0x00, 0x00, 0x00};
BYTE offline_patch_repl[] = {0xB8, 0x00, 0x00, 0x00, 0x00, 0x90};

std::string offline_patch_templ_mask = "xx....xx....xx....xx....xx";
BYTE offline_patch_templ_find[] = {0x89, 0xB3, 0x00, 0x00, 0x00, 0x00, 0x89, 0xB3, 0x00, 0x00, 0x00, 0x00, 0x89, 0xB3, 0x00, 0x00, 0x00, 0x00, 0x89, 0xB3, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x15};

BYTE offline_btest[] = {0x8B, 0x10, 0x8B, 0x52, 0x70, 0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x01};

class OfflinePatch : public Patch {
private:
	std::vector<Address> patched_addrs;
	std::vector<BYTE*> original_values;
	bool is_valid;

public:
	~OfflinePatch() {
		for(BYTE* ptr : original_values) {
			delete[] ptr;
		}
	}

	OfflinePatch(std::shared_ptr<Spelunky> spel) : Patch(spel), is_valid(true)
	{
		BYTE offline_patch_each[sizeof(offline_patch_each_template)];
		{
			std::memcpy(offline_patch_each, offline_patch_each_template, sizeof(offline_patch_each));
			Address fill_addr = spel->find_mem(offline_patch_templ_find, offline_patch_templ_mask);
			if(fill_addr != 0) {
				fill_addr += 0x18;
				spel->read_mem(fill_addr, offline_patch_each, sizeof(offline_patch_each));
				
#ifdef DEBUG_MODE
				std::cout << "Found offline patch template fill at " << std::setbase(16) << fill_addr << std::endl;
#endif
			}
			else 
			{
#ifdef DEBUG_MODE
				std::cout << "Warning: Could not find offline patch template fill." << std::endl;
#endif
				return;
			}
		}

		BYTE buf[sizeof(offline_btest)];

		Address current = 0x0;
		while((current = spel->find_mem(offline_patch_each, offline_patch_mask, current)) > 0) {
			spel->read_mem(current+12, buf, sizeof(offline_btest));

			if(std::equal(buf, buf+sizeof(offline_btest), offline_btest)) {
				current++;
				continue;
			}

			BYTE* buf2 = new BYTE[sizeof(offline_patch_repl)];
			spel->read_mem(current, buf2, sizeof(offline_patch_repl));
			
#ifdef DEBUG_MODE
			std::cout << "Offline patch at " << std::setbase(16) << current << std::endl;
#endif

			original_values.push_back(buf2);
			patched_addrs.push_back(current);
			current++;
		}
		
		if(patched_addrs.empty()) {
			is_valid = false;
			return; //If these are empty, either GOG or a Spelunky patch broke something!
			//throw std::runtime_error("Unable to find offsets for offline patch.");
		}
	}
	
	virtual bool _undo() override {
		//int idx = 0;
		//for(Address addr : patched_addrs) {
		//	BYTE* orig = original_values[idx];
		//	spel->write_mem(addr, orig, sizeof(offline_patch_repl));
		//	idx++;
		//}
		return true;
	}

	virtual bool _perform() override {
		for(Address addr : patched_addrs) {
			spel->write_mem(addr, offline_patch_repl, sizeof(offline_patch_repl));
		}

		return true;
	}

public:
	virtual bool valid() override {
		return is_valid;
	}
};