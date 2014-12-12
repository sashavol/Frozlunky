#include "tile_editing_hintbar.h"
#include "tile_description.h"

#include <string>
#include <sstream>
#include <iostream>


TileEditingHintbar::TileEditingHintbar(int x, int y, int w, int h) : 
	Fl_Widget(x, y, w, h),
	tile(0), 
	area(MINES), 
	chunk(nullptr)
{
	this->box(Fl_Boxtype::FL_BORDER_BOX);
}

void TileEditingHintbar::update_label() {
	std::ostringstream oss;

	if(chunk) {
		oss << "Chunk: " << Description::ChunkDescription(chunk) << "  ";
	}

	if(tile) {
		//derive render mode from given by default, otherwise derive from chunk attributes
		if(area != AreaRenderMode::INVALID) {
			oss << "Tile: " << Description::TileDescription(tile, area);
		}
		else {
			oss << "Tile: " << Description::TileDescription(tile, this->chunk);
		}
	}

	text = oss.str();
}

void TileEditingHintbar::set_tile(char tile, AreaRenderMode area) {
	this->tile = tile;
	this->area = area;

	update_label();
}

void TileEditingHintbar::set_chunk(Chunk* chunk) {
	this->chunk = chunk;
	update_label();
}

int TileEditingHintbar::handle(int evt) {
	return 0;
}

void TileEditingHintbar::draw() {
	fl_draw_box(Fl_Boxtype::FL_UP_BOX, x(), y(), w(), h(), parent()->color());
	fl_color(0x0);
	fl_font(FL_HELVETICA, FL_NORMAL_SIZE);
	fl_draw(text.c_str(), x()+5, y(), w(), h(), FL_ALIGN_LEFT);
}