#include "custom_hud.h"
#include <cmath>

#define INIT_SIZE 256
#define EXEC_SIZE 128
#define VARS_SIZE 64

//+7
//BYTE from_find[] = {0xDD, 0xD8, 0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0x6A, 0x00, 0x6A, 0x0D, 0x8B};
//std::string from_mask = "xxx.....x....xxxxx";

BYTE from_find[] = {0x8B,0xCC,0x8B,0xCC,0x04,0xFF,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xE8,0xCC,0xCC,0xCC,0xCC,0x8B};
std::string from_mask = "x.x.xx.x.....x....x";

// A (+4,+59)  -> g_CurrentGamePtr
// B (+13)     -> coord_args[0]
// C (+27)     -> coord_args[1]
// D (+40)     -> coord_args[2]
// E (+48)     -> &alloc_string
// F (+53)     -> diff(CreateGraphicsFromText)
// 1 (+68)     -> diff(gfx_RenderGraphic)

BYTE exec_code[] = {0x50, 0x51, 0x57, 0xA1, 
	0xAA, 0xAA, 0xAA, 0xAA, 0x8B, 0x78, 0x50, 0xD9, 0x05, 
	0xBB, 0xBB, 0xBB, 0xBB, 0x6A, 0x02, 0x51, 0xD9, 0x1C, 0x24, 0x6A, 0x01, 0xD9, 0x05, 
	0xCC, 0xCC, 0xCC, 0xCC, 0x83, 0xEC, 0x08, 0xD9, 0x5C, 0x24, 0x04, 0xD9, 0x05, 
	0xDD, 0xDD, 0xDD, 0xDD, 0xD9, 0x1C, 0x24, 0x68, 
	0xEE, 0xEE, 0xEE, 0xEE, 0xE8, 
	0xFF, 0xFF, 0xFF, 0xFF, 0x50, 0xA1, 
	0xAA, 0xAA, 0xAA, 0xAA, 0x8B, 0x48, 0x50, 0x51, 0xE8, 
	0x11, 0x11, 0x11, 0x11, 0x5F, 0x59, 0x58};


//+15
BYTE cgft_find[] = {0x2C, 0xD9, 0x05, 0xFF, 0xFF, 0xFF, 0xFF, 0xD9, 0x1C, 0x24, 0x51, 0xE8,
	0x00, 0x00, 0x00, 0x00, 0xA1, 0xFF, 0xFF, 0xFF, 0xFF,
	0x83};
std::string cgft_mask = "xxx....xxxxx....x....x";

//+17
BYTE grg_find[] = {0x8B, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x89, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0x8B, 0xFF, 0x50, 0xFF, 0xE8, 0xFF, 0xFF, 0xFF,
	0xFF, 0xA1, 0xFF, 0xFF, 0xFF, 0xFF, 0x8B};
std::string grg_mask = "x.....x.....x.x.x....x....x";

#define JUMP_PATCH_SIZE 5

					//size   y     x
float coord_args[] = {0.5f, 10.5f, 0.5f};



CustomHudPatch::CustomHudPatch(std::shared_ptr<DerandomizePatch> dp, std::wstring text) : 
	Patch(dp->spel), 
	dp(dp),
	current_text(text), 
	allocated(NULL), 
	allocated_size(0),
	executable_space(NULL),
	vars_space(NULL),
	jump_patch_orig(nullptr),
	jump_patch_addr(0),
	is_valid(true)
{
	CreateGraphicFromText = spel->get_stored_hook("cgft");
	if(CreateGraphicFromText == 0x0) {
		Address addr = spel->find_mem(cgft_find, cgft_mask);
		if(addr == 0x0) {
			is_valid = false;
			#ifdef DEBUG_MODE
			std::cout << "[HUD] Failed to find CreateGraphicFromText" << std::endl;
			#endif
			return;
		}
		addr += 12;
		spel->read_mem(addr, &CreateGraphicFromText, sizeof(Address));
		CreateGraphicFromText += addr+4;

#ifdef DEBUG_MODE
		std::cout << "Found CreateGraphicFromText at " << std::setbase(16) << CreateGraphicFromText << std::endl;
#endif

		spel->store_hook("cgft", CreateGraphicFromText);
	}

	gfx_RenderGraphic = spel->get_stored_hook("grg");
	if(gfx_RenderGraphic == 0x0) {
		Address addr = spel->find_mem(grg_find, grg_mask);
		if(addr == 0x0) {
			is_valid = false;
			#ifdef DEBUG_MODE
			std::cout << "[HUD] Failed to find gfx_RenderGraphic" << std::endl;
			#endif
			return;
		}
		addr += 17;

		spel->read_mem(addr, &gfx_RenderGraphic, sizeof(Address));
		gfx_RenderGraphic += addr+4;

#ifdef DEBUG_MODE
		std::cout << "Found gfx_RenderGraphic at " << std::setbase(16) << " (*" << addr << ") " << std::setbase(16) << gfx_RenderGraphic << std::endl;
#endif
		spel->store_hook("grg", gfx_RenderGraphic);
	}

	jump_patch_addr = spel->get_stored_hook("hdjmp");
	if(jump_patch_addr == 0x0) {
		jump_patch_addr = spel->find_mem(from_find, from_mask);
		if(jump_patch_addr == 0x0) {
			is_valid = false;
			#ifdef DEBUG_MODE
			std::cout << "[HUD] Failed to find jump_patch_addr" << std::endl;
			#endif
			return;
		}
		spel->store_hook("hdjmp", jump_patch_addr);
	}

	jump_patch_orig = new BYTE[JUMP_PATCH_SIZE];
	spel->read_mem(jump_patch_addr, jump_patch_orig, JUMP_PATCH_SIZE);


	allocated = spel->allocate(INIT_SIZE);
	if(allocated == NULL) {
		is_valid = false;
		return;
	}
	allocated_size = INIT_SIZE;

	vars_space = spel->allocate(VARS_SIZE);
	if(vars_space == NULL) {
		is_valid = false;
		return;
	}

	executable_space = spel->allocate(EXEC_SIZE, true);
	if(executable_space == NULL) {
		is_valid = false;
		return;
	}

#ifdef DEBUG_MODE
	std::cout << "[HUD] Allocated executable space at " << std::setbase(16) << executable_space << std::endl;
	std::cout << "[HUD] Allocated variable space at " << std::setbase(16) << vars_space << std::endl;
	std::cout << "[HUD] Allocated string space at " << std::setbase(16) << allocated << std::endl;
	std::cout << "Jump patch to executable space will be created at " << std::setbase(16) << jump_patch_addr << std::endl;
#endif

	set_text(text);
}

CustomHudPatch::~CustomHudPatch() {
	if(allocated != NULL) {
		this->_undo();
		spel->release(allocated);
		allocated = NULL;
		allocated_size = 0;
	}

	if(vars_space != NULL) {
		spel->release(vars_space);
		vars_space = NULL;
	}

	if(executable_space != NULL) {
		spel->release(executable_space);
		executable_space = NULL;
	}
	
	if(jump_patch_orig != nullptr) {
		delete[] jump_patch_orig;
		jump_patch_orig = nullptr;
	}
}


bool CustomHudPatch::_undo() {
	spel->write_mem(jump_patch_addr, jump_patch_orig, JUMP_PATCH_SIZE);
	return true;
}

int diff(Address from, Address to) {
	return to - (from+0x5);
}

bool CustomHudPatch::_perform() {
	spel->write_mem(vars_space, coord_args, sizeof(coord_args));
	spel->write_mem(executable_space, exec_code, sizeof(exec_code));
	
	Address g_CurrentGamePtr = dp->game_ptr();
	spel->write_mem(executable_space+4, &g_CurrentGamePtr, sizeof(Address));
	spel->write_mem(executable_space+59, &g_CurrentGamePtr, sizeof(Address));

	Address coord_args = vars_space;
	spel->write_mem(executable_space+13, &coord_args, sizeof(Address)); coord_args += sizeof(float);
	spel->write_mem(executable_space+27, &coord_args, sizeof(Address)); coord_args += sizeof(float);
	spel->write_mem(executable_space+40, &coord_args, sizeof(Address));
	
	spel->write_mem(executable_space+48, &allocated, sizeof(Address));

	int diff_cgft = diff(executable_space+52, CreateGraphicFromText);
	int diff_grg = diff(executable_space+67, gfx_RenderGraphic);
	spel->write_mem(executable_space+53, &diff_cgft, sizeof(int));
	spel->write_mem(executable_space+68, &diff_grg, sizeof(int));

	spel->jmp_build(jump_patch_addr, JUMP_PATCH_SIZE, executable_space, sizeof(exec_code));
	return true;
}

void CustomHudPatch::set_text(const std::wstring& text) 
{
	if(!is_valid)
		return;

	size_t text_len = text.size()*sizeof(wchar_t);
	if(text_len > allocated_size) 
	{
		bool reinitiate = false;
		if(this->is_active()) {
			this->_undo();
			reinitiate = true;
		}
		
		size_t target = (size_t)std::pow(2, 1 + (int)(std::log(text_len)/std::log(2)));
		
		spel->release(allocated);
		allocated = NULL; allocated_size = 0;
		
		allocated = spel->allocate(target);
		if(allocated == NULL) {
			is_valid = false;
			return;
		}
		allocated_size = target;

#ifdef DEBUG_MODE
		std::cout << "Reallocated string space at " << allocated << std::endl;
#endif

		if(reinitiate) {
			this->_perform();
		}
	}

	wchar_t end_bytes = 0;
	spel->write_mem(allocated, text.c_str(), text_len);
	spel->write_mem(allocated+text_len, &end_bytes, sizeof(wchar_t));
}