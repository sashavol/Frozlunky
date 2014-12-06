#include "tile_draw.h"

Fl_Color tile_color(char tile) {
	switch(tile) {
	case 'w': //water
		return 0xAAAAFF00;
	case 'e': //bee-hive inner
		return 0xFFFF2000;
	case 'z': //bee-hive outer
		return 0xFF905000;
	case '#': //tnt
		return 0xFF805000;
	case '?': //?
		return 0xFFFFFF00;
	case '4': //movable push block
		return 0x95857500;
	case 'C': //non-movable push block
		return 0x786D5400;
	case '*': //mace
		return 0x60505300;
	case 'd': //jungle terrain
		return 0xADFF2F00;
	case 'I': //idol
		return 0xCC990000;
	case '~': //bounce trap
		return 0x79DC7900;
	case 'v': //wooden platform (usually)
		return 0x8B451300;
	case 'G': //ladder (in jungle)
		return 0xCB873600;
	case 'g': //coffin spawn
		return 0x99004C00;
	case 'a': //ankh
		return 0xE2B22700;
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