#pragma once

#include "items.h"
#include "patches.h"
#include "derandom.h"
#include <vector>

#define SHOP_TEXT_LENGTH 18

struct ShopText {
	wchar_t str[SHOP_TEXT_LENGTH];
};

class ShopContentsPatch : public Patch {
private:
	bool is_valid;

	std::shared_ptr<DerandomizePatch> dp;
	std::vector<ItemID> items;
	unsigned levels_mask;

	BYTE* insert_jmp_orig;

	std::vector<Address> shoptext_addrs;
	std::vector<ShopText> shoptext_origs;
	Address insert_jmp;
	Address exec_alloc;

	void patch_shop_text();
	void restore_shop_text();

	virtual bool _perform() override;
	virtual bool _undo() override;

public:
	typedef std::vector<ItemID> item_storage;

	ShopContentsPatch(std::shared_ptr<DerandomizePatch> dp);
	~ShopContentsPatch();

	virtual bool valid() override {
		return is_valid;
	}

	unsigned levels() {
		return levels_mask;
	}

	void set_levels(unsigned mask);

	item_storage get_items() {
		return items;
	}

	void set_items(item_storage& items);
};