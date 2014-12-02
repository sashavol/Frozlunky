#pragma once

#include "tile_chunk.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>

Fl_Color tile_color(char tile);
void draw_tile(char tile, int x, int y, int w, int h);