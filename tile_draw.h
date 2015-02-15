#pragma once

#include "tile_chunk.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>

enum AreaRenderMode {
	TUTORIAL = 0,
	MINES = 1,
	JUNGLE = 2, 
	HAUNTED_CASTLE = 3, 
	WORM = 4, 
	ICE_CAVES = 5, 
	MOTHERSHIP = 6,
	TEMPLE = 8,
	HELL = 9,
	INVALID = 10
};

AreaRenderMode mode_from_chunk(Chunk* cnk);
AreaRenderMode mode_from_name(const std::string& area);
Fl_Color tile_color(char tile);
void draw_tile(char tile, int x, int y, int w, int h, AreaRenderMode arm=MINES);

inline Fl_Color entity_color(int entity);
void draw_entity(int entity, int x, int y, int w, int h, bool message_entity = false);