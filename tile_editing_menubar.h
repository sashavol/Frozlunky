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
		
		KeyTrigger(int key, bool ctrl, bool shift, bool alt, const std::string& hotkey_text);
	};

	typedef std::function<void(KeyTrigger)> key_handler;

private:
	key_handler handler;
	std::vector<std::function<void(Fl_Widget*)>*> heap_functions;

private:
	TileEditingMenuBar(const TileEditingMenuBar& o);
	TileEditingMenuBar(TileEditingMenuBar&& o);
	TileEditingMenuBar& operator=(const TileEditingMenuBar& o);
	TileEditingMenuBar& operator=(TileEditingMenuBar&& o);

public:
	TileEditingMenuBar(int x, int y, int w, int h, key_handler handler);
	~TileEditingMenuBar();
};