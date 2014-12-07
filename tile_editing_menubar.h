#pragma once

#include <FL/Fl_Menu_Bar.H>
#include "tile_editor_widget.h"
#include <functional>

class TileEditingMenuBar : public Fl_Menu_Bar {
public:
	struct KeyTrigger {
		bool shift;
		bool ctrl;
		bool alt;
		int key;

		std::string hotkey_text;
		
		KeyTrigger(int key, bool shift, bool ctrl, bool alt, const std::string& hotkey_text);
	};

	typedef std::function<void(KeyTrigger)> key_handler;

private:
	key_handler handler;

public:
	TileEditingMenuBar(int x, int y, int w, int h, key_handler handler);
};