#include "tile_picker.h"

TilePicker::TilePicker(AreaRenderMode arm, const std::set<char>& tiles, int x, int y, int w, int h, int xu, int yu)
	:   tiles(tiles), 
		x(x), 
		y(y), 
		w(w), 
		h(h), 
		xu(xu), 
		yu(yu), 
		selected(0),
		entity_selected(0),
		arm(arm)
	{}

void TilePicker::resize(int x, int y, int w, int h, int xu, int yu) {
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	this->xu = xu;
	this->yu = yu;
}

void TilePicker::draw() {
	int px = x, py = y;
	for(char c : tiles) {
		draw_tile(c, px, py, xu, yu, arm);

		if(c == selected) {
			fl_rect(px, py, xu, yu, 0xCF000000);
			fl_rect(px+1, py+1, xu-2, yu-2, 0xCF000000);
		}

		px += xu;
		if(px+xu >= x+w) {
			py += yu;
			px = x;
		}
	}

	for(int entity : recent_entities) {
		draw_entity(entity, px, py, xu, yu);

		if(entity == entity_selected) {
			fl_rect(px, py, xu, yu, 0xCF000000);
			fl_rect(px+1, py+1, xu-2, yu-2, 0xCF000000);
		}

		px += xu;
		if(px+xu >= x+w) {
			py += yu;
			px = x;
		}
	}
}

char TilePicker::tile(int rx, int ry) {
	int px = x, py = y;
	for(char c : tiles) {
		if(rx >= px && rx < px+xu && ry >= py && ry < py+yu) {
			return c;
		}
			
		px += xu;
		if(px+xu >= x+w) {
			py += yu;
			px = x;
		}
	}
	return 0;
}

int TilePicker::entity(int rx, int ry) {
	int px = x, py = y;
	for(char c : tiles) {
		px += xu;
		if(px+xu >= x+w) {
			py += yu;
			px = x;
		}
	}

	for(int entity : recent_entities) {
		if(rx >= px && rx < px+xu && ry >= py && ry < py+yu) {
			return entity;
		}
			
		px += xu;
		if(px+xu >= x+w) {
			py += yu;
			px = x;
		}
	}

	return 0;
}

std::pair<int, int> TilePicker::elem_position(char tile, int entity) {
	int px = x, py = y;

	for(char c : tiles) {
		if(c == tile) {
			return std::make_pair(px, py);
		}

		px += xu;
		if(px+xu >= x+w) {
			py += yu;
			px = x;
		}
	}
		
	for(int e : recent_entities) {
		if(e == entity) {
			return std::make_pair(px, py);
		}
		
		px += xu;
		if(px+xu >= x+w) {
			py += yu;
			px = x;
		}
	}
		
	return std::make_pair(-1, -1);
}

void TilePicker::try_move(int dx, int dy) {
	std::pair<int,int> pos = elem_position(tile(), entity());
	if(pos.first == -1)
		return;

	int xn = pos.first + xu*dx, yn = pos.second + yu*dy;
		
	int et = entity(xn, yn);
	char tl = tile(xn, yn);

	if(tl != 0) {
		select(tl);
		return;
	}

	if(et != 0) {
		entity_select(et);
		return;
	}
}

char TilePicker::tile() {
	return selected;
}

int TilePicker::entity() {
	return entity_selected;	
}

void TilePicker::entity_select(int entity) {
	if(std::find(recent_entities.begin(), recent_entities.end(), entity) == recent_entities.end()) {
		recent_entities.insert(recent_entities.begin(), entity);
		if(recent_entities.size() > MAX_RECENT_ENTITIES) {
			recent_entities.pop_back();
		}
	}

	selected = 0;
	entity_selected = entity;
}

std::vector<int>& TilePicker::get_recent_entities() {
	return recent_entities;
}

void TilePicker::select(char tile) {
	entity_selected = 0;
	selected = tile;
}

void TilePicker::unselect() {
	selected = 0;
	entity_selected = 0;
}