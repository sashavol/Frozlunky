#pragma once

#include <map>
#include <string>
#include <functional>

#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>

#include "patches.h"
#include "patch_group.h"
#include "derandom.h"
#include "shop_contents_patch.h"
#include "custom_hud.h"
#include "game_hooks.h"


#define BUTTON_CLASS(NAME) \
class NAME : public Fl_Button { \
public: \
	NAME(int x, int y, int w, int h, char* L) : Fl_Button(x,y,w,h,L) {} \
	virtual int handle(int evt) override; \
}

namespace Mods {
	typedef std::function<void(bool)> visibility_callback;
	
	BUTTON_CLASS(DoneButton);

	class ModCheckbox : public Fl_Check_Button {
	public:
		std::string target;

	public:
		ModCheckbox(const std::string& target, int x, int y, int w, int h, const char* label);
	};

	class ShopItemCheckbox : public Fl_Check_Button {
	public:
		ItemID id;

	public:
		ShopItemCheckbox(ItemID id, int x, int y, int w, int h, const char* label);
	};

	BUTTON_CLASS(RemoveAIButton);
	BUTTON_CLASS(AddAIButton);

	std::shared_ptr<PatchGroup> ModsGroup();
	void ShowModsGUI();
	void HideModsGUI();
	void SetVisibilityChangeCallback(visibility_callback cb);
	void Initialize(std::shared_ptr<DerandomizePatch> dp, std::shared_ptr<GameHooks> gh, std::shared_ptr<CustomHudPatch> chp);
}

#undef BUTTON_CLASS
#undef CHECK_CLASS