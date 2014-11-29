#pragma once

#include "patches.h"
#include "derandom.h"
#include "tile_patch.h"
#include "seeder.h"
#include "game_hooks.h"
#include "static_chunk_patch.h"
#include <functional>
#include <vector>

#define BUTTON_CLASS(NAME, LABEL) \
class NAME : public Fl_Button { \
public: \
	NAME(int x, int y, int w, int h) : Fl_Button(x,y,w,h,LABEL) {} \
	virtual int handle(int evt) override; \
}

namespace TileEditing {
	bool Initialize(std::shared_ptr<Seeder> seeder, std::shared_ptr<StaticChunkPatch> scp);
	bool Valid();
	bool Visible();
	void DisplayStateCallback(std::function<void(bool)> cb);
	void ShowUI();
	void HideUI();
	std::shared_ptr<::Patch> GetPatch();
}