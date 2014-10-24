#include "shop_contents_patch.h"
#include <iostream>

#define SHOP_EXEC_ALLOC 256
#define SHOP_INSERT_JMP_SIZE 5

BYTE shop_insertjmp_find[] = {0x80,0x7C,0x24,0xCC,0x00,0x0F,0x85,0xCC,0xCC,0xCC,0xCC,0xE8,0xCC,0xCC,0xCC,0xCC,0x33,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xF7};
std::string shop_insertjmp_mask = "xxx.xxx....x....x......x";

/*
	push eax
	push ecx
	mov eax,[AAAAAAAA] (+3)
	mov ecx,[eax+BBBBBBBB] (+9)
	xor eax,eax
	inc eax
	shl eax,cl
	and eax,CCCCCCCC (+19)
	test eax,eax
	je DDDDDDDD (+27)
	mov byte ptr [esp+13], 1 (+34)
*/
BYTE shop_exec_init_code[] = {
	0x50,0x51,
	0xA1,0xAA,0xAA,0xAA,0xAA,
	0x8B,0x88,0xBB,0xBB,0xBB,0xBB,
	0x31,0xC0,0x40,0xD3,0xE0,
	0x25,0xCC,0xCC,0xCC,0xCC,
	0x85,0xC0,
	0x0F,0x84,0xDD,0xDD,0xDD,0xDD
};

/*
	cmp ebp, AA (+2)
	jne eip+5
	mov esi, BBBBBBBB (+6)
*/
BYTE shop_exec_item_slot[] = {
	0x83,0xFD,0xAA,
	0x75,0x05,0xBE,0xBB,0xBB,0xBB,0xBB};

//pop ecx; pop eax
BYTE shop_exec_end_code[] = {0x59, 0x58};

wchar_t shoptext_find[] =  L"IDS_SHOPWELCOME";
std::string shoptext_mask = "xxxxxxxxxxxxxxx" "xxxxxxxxxxxxxxx";
wchar_t shoptext_repl[] = L"IDS_JOURNAL_ITEMS";

ShopContentsPatch::ShopContentsPatch(std::shared_ptr<DerandomizePatch> dp) : 
	Patch(dp->spel),
	dp(dp),
	is_valid(true),
	levels_mask(0xFFFFFFFF),
	exec_alloc(NULL),
	insert_jmp_orig(nullptr)
{
	exec_alloc = spel->allocate(SHOP_EXEC_ALLOC, true);
	if(!exec_alloc) {
		is_valid = false;

#ifdef DEBUG_MODE
		std::cout << "[SHOP] Error allocating executable space" << std::endl;
#endif
		return;
	}

	insert_jmp = spel->find_mem(shop_insertjmp_find, shop_insertjmp_mask);
	if(insert_jmp == 0x0) {
		is_valid = false;
#ifdef DEBUG_MODE
		std::cout << "[SHOP] Failed to find insert offset" << std::endl;
#endif
		return;
	}

#ifdef DEBUG_MODE
	std::cout << "[SHOP] Inserting jump at "<< std::setbase(16) << insert_jmp << std::endl;
	std::cout << "[SHOP] Allocated executable space at "<< std::setbase(16) << exec_alloc << std::endl;
#endif

	insert_jmp_orig = new BYTE[SHOP_INSERT_JMP_SIZE];
	spel->read_mem(insert_jmp, insert_jmp_orig, SHOP_INSERT_JMP_SIZE);

	{
		Address start = 0;
		while((start = spel->find_mem((BYTE*)shoptext_find, shoptext_mask, start)) != 0x0) {
			ShopText st;
			spel->read_mem(start, st.str, SHOP_TEXT_LENGTH*sizeof(wchar_t));
			shoptext_addrs.push_back(start);
			shoptext_origs.push_back(st);
			start++;
		}

#ifdef DEBUG_MODE
		std::cout << "Found " << shoptext_addrs.size() << " shop text addresses." << std::endl;
#endif
	}
}

ShopContentsPatch::~ShopContentsPatch() {
	if(exec_alloc != NULL) {
		this->_undo();
		spel->release(exec_alloc);
	}

	if(insert_jmp_orig != nullptr) {
		delete[] insert_jmp_orig;
	}
}

void ShopContentsPatch::set_levels(unsigned mask) {
	this->levels_mask = mask;

	if(this->is_active()) {
		this->_perform();
	}
}

void ShopContentsPatch::patch_shop_text() {
	for(Address addr : shoptext_addrs) {
		spel->write_mem(addr, shoptext_repl, sizeof(shoptext_repl), true);
	}
}

void ShopContentsPatch::restore_shop_text() {
	auto iter = shoptext_origs.begin();
	for(Address addr : shoptext_addrs) {
		spel->write_mem(addr, iter->str, SHOP_TEXT_LENGTH*sizeof(wchar_t), true);
		iter++;
	}
}

bool ShopContentsPatch::_perform() {
	if(this->is_active()) {
		this->_undo();
	}

	patch_shop_text();

	unsigned offset = 0;

	{
		Address game_ptr = dp->game_ptr();
		Address level_offs = dp->current_level_offset();
		Address end = exec_alloc+sizeof(shop_exec_init_code)+items.size()*sizeof(shop_exec_item_slot);
		unsigned diff = je_diff(exec_alloc+25, end);

		spel->write_mem(exec_alloc, shop_exec_init_code, sizeof(shop_exec_init_code));
		spel->write_mem(exec_alloc+3, &game_ptr, sizeof(Address));
		spel->write_mem(exec_alloc+9, &level_offs, sizeof(Address));
		spel->write_mem(exec_alloc+19, &levels_mask, sizeof(unsigned));
		spel->write_mem(exec_alloc+27, &diff, sizeof(unsigned));
		offset += sizeof(shop_exec_init_code);
	}

	BYTE idx = 0;
	for(ItemID item : items) {
		spel->write_mem(exec_alloc+offset, shop_exec_item_slot, sizeof(shop_exec_item_slot));
		spel->write_mem(exec_alloc+offset+2, &idx, sizeof(BYTE));
		spel->write_mem(exec_alloc+offset+6, &item, sizeof(ItemID));
		idx++;
		offset += sizeof(shop_exec_item_slot);
	}

	spel->write_mem(exec_alloc+offset, shop_exec_end_code, sizeof(shop_exec_end_code));
	offset += sizeof(shop_exec_end_code);

	spel->jmp_build(insert_jmp, SHOP_INSERT_JMP_SIZE, exec_alloc, offset);

	//crate fix
	BYTE one = 1;
	spel->write_mem(exec_alloc+offset+4, &one, sizeof(BYTE));

	return true;
}

bool ShopContentsPatch::_undo() {
	restore_shop_text();
	spel->write_mem(insert_jmp, insert_jmp_orig, SHOP_INSERT_JMP_SIZE);
	return true;
}

void ShopContentsPatch::set_items(item_storage& items) {
	this->items = items;

	if(this->is_active()) {
		this->_perform();
	}
}