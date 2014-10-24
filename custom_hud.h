#pragma once

#include "patches.h"
#include "derandom.h"

#include <string>

//!TODO (offtopic) check if death offset null and invalidate daily if that can't be found


class CustomHudPatch : public Patch {
private:
	std::shared_ptr<DerandomizePatch> dp;
	
	Address allocated;
	Address vars_space;
	Address executable_space;

	Address gfx_RenderGraphic;
	Address CreateGraphicFromText;
	
	Address jump_patch_addr;
	BYTE* jump_patch_orig;
	
	size_t allocated_size;
	bool is_valid;

	std::wstring current_text;

private:
	virtual bool _perform() override;
	virtual bool _undo() override;

public:
	CustomHudPatch(std::shared_ptr<DerandomizePatch> spel, std::wstring text=std::wstring());
	~CustomHudPatch();

	virtual bool valid() override {
		return is_valid;
	}

	void set_text(const std::wstring& text);
};