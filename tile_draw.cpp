#include "tile_draw.h"

Fl_Color tile_color(char tile) {
	switch(tile) {
	case 'w': //water
		return 0xAAAAFF00;
	case 'e': //bee-hive inner
		return 0xFFFF2000;
	case 'z': //bee-hive outer
		return 0xFF905000;
	default:
		return ((tile - '0')*0x23456721) & 0xFFFFFF00;
	}
}

void draw_tile(char tile, int x, int y, int w, int h) {
	Fl_Color ctile = tile_color(tile);
	char str[] = {tile, 0};

	fl_draw_box(Fl_Boxtype::FL_FLAT_BOX, x, y, w, h, fl_darker(ctile));
	fl_draw_box(Fl_Boxtype::FL_FLAT_BOX, x, y, w-1, h-1, ctile);

	fl_color(0);
	fl_draw(str, x, y, w, h, FL_ALIGN_INSIDE);
}