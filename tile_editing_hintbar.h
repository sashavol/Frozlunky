#pragma once

#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>

#include "tile_draw.h"
#include "tile_chunk.h"

class TileEditingHintbar : public Fl_Group {
private:
	char tile;
	AreaRenderMode area;

	Chunk* chunk;

public:
	TileEditingHintbar(int x, int y, int w, int h) : 
		Fl_Group(x, y, w, h),
		tile(0), 
		area(MINES), 
		chunk(nullptr)
	{}
	
	void set_tile(char tile, AreaRenderMode area);
	void set_chunk(Chunk* chunk);

	virtual void draw() override;
};