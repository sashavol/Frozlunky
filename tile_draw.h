#pragma once

#include "tile_chunk.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>

enum AreaRenderMode {
	MINES, JUNGLE, ICE_CAVES, TEMPLE, HELL, WORM
};

AreaRenderMode mode_from_name(const std::string& area);
Fl_Color tile_color(char tile);
void draw_tile(char tile, int x, int y, int w, int h, AreaRenderMode arm=MINES);