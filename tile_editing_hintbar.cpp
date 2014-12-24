#include "tile_editing_hintbar.h"
#include "tile_description.h"
#include "known_entities.h"

#include <string>
#include <sstream>
#include <iostream>


TileEditingHintbar::TileEditingHintbar(int x, int y, int w, int h) : 
	Fl_Widget(x, y, w, h),
	tile(0), 
	area(MINES), 
	chunk(nullptr),
	entity(0)
{
	this->box(Fl_Boxtype::FL_BORDER_BOX);
}


void TileEditingHintbar::update_label() {
	std::ostringstream oss;

	if(chunk) {
		oss << "Chunk: " << Description::ChunkDescription(chunk) << "  ";
		
		if(entity) {
			oss << "Entity: " << KnownEntities::GetName(entity);
		}
		else if(tile) {
			//derive render mode from given by default, otherwise derive from chunk attributes
			if(area != AreaRenderMode::INVALID) {
				oss << "Tile: " << Description::TileDescription(tile, area);
			}
			else {
				oss << "Tile: " << Description::TileDescription(tile, this->chunk);
			}
		}
	}
	else if(entity) {
		oss << "Picked Entity: " << KnownEntities::GetName(entity);
	}
	else if(tile) {
		oss << "Picked Tile: " << Description::TileDescription(tile, area);
	}

	text = oss.str();
}

void TileEditingHintbar::set_tile(char tile, AreaRenderMode area, Chunk* parent) {
	this->tile = tile;
	this->area = area;
	this->chunk = parent;
	this->entity = 0;

	update_label();
}

int TileEditingHintbar::handle(int evt) {
	return 0;
}

void TileEditingHintbar::set_entity(int entity, Chunk* parent) {
	this->tile = 0;
	this->entity = entity;
	this->chunk = parent;

	update_label();
}

void TileEditingHintbar::draw() {
	fl_draw_box(Fl_Boxtype::FL_UP_BOX, x(), y(), w(), h(), parent()->color());
	fl_color(0x0);
	fl_font(FL_HELVETICA, FL_NORMAL_SIZE);
	fl_draw(text.c_str(), x()+5, y(), w(), h(), FL_ALIGN_LEFT);
}