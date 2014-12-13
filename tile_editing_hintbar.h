#pragma once

#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>

#include "tile_draw.h"
#include "tile_chunk.h"

class TileEditingHintbar : public Fl_Widget {
private:
	std::string text;

	char tile;
	AreaRenderMode area;

	Chunk* chunk;

	void update_label();

public:
	TileEditingHintbar(int x, int y, int w, int h);

	void set_tile(char tile, AreaRenderMode area, Chunk* parent);
	
	virtual void draw() override;
	virtual int handle(int evt) override;
};