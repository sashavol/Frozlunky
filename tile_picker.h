#pragma once

#include "tile_draw.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <set>

class TilePicker {
private:
	int x, y, w, h, xu, yu;
	std::set<char> tiles;
	char selected;

public:
	TilePicker(const std::set<char>& tiles, int x, int y, int w, int h, int xu, int yu)
	:   tiles(tiles), 
		x(x), 
		y(y), 
		w(w), 
		h(h), 
		xu(xu), 
		yu(yu), 
		selected(0)
	{}

	void draw() {
		int px = x, py = y;
		for(char c : tiles) {
			draw_tile(c, px, py, xu, yu);

			if(c == selected) {
				fl_rect(px, py, xu, yu, 0xFF000000);
			}

			px += xu;
			if(px > x+w) {
				py += yu;
				px = x;
			}
		}
	}

	char tile(int rx, int ry) {
		int px = x, py = y;
		for(char c : tiles) {
			if(rx >= px && rx < px+xu && ry >= py && ry < py+yu) {
				return c;
			}
			
			px += xu;
			if(px > x+w) {
				py += yu;
				px = x;
			}
		}
		return 0;
	}

	void select(char tile) {
		selected = tile;
	}

	void unselect() {
		selected = 0;
	}
};