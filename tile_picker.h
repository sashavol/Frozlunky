#pragma once

#include "tile_draw.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <set>

#define MAX_RECENT_ENTITIES 10

class TilePicker {
private:
	int x, y, w, h, xu, yu;
	std::set<char> tiles;
	std::vector<int> recent_entities;
	char selected;
	int entity_selected;
	AreaRenderMode arm;

public:
	TilePicker(AreaRenderMode arm, const std::set<char>& tiles, int x, int y, int w, int h, int xu, int yu)
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

	void draw() {
		int px = x, py = y;
		for(char c : tiles) {
			draw_tile(c, px, py, xu, yu, arm);

			if(c == selected) {
				fl_rect(px, py, xu, yu, 0xCF000000);
				fl_rect(px+1, py+1, xu-2, yu-2, 0xCF000000);
			}

			px += xu;
			if(px > x+w) {
				py += yu;
				px = x;
			}
		}
		
		for(int entity : recent_entities) {
			draw_entity(entity, px, py, xu, yu);

			if(entity == selected) {
				fl_rect(px, py, xu, yu, 0xCF000000);
				fl_rect(px+1, py+1, xu-2, yu-2, 0xCF000000);
			}

			px += xu;
			if(px > x+w) {
				py += yu;
				px = x;
			}
		}
	}

	char tile(int rx, int ry) {
		int px = x, py = y;
		for(char c : tiles) {
			if(rx >= px && rx < px+xu && ry >= py && ry < py+yu) {
				return c;
			}
			
			px += xu;
			if(px > x+w) {
				py += yu;
				px = x;
			}
		}
		return 0;
	}

	int entity(int rx, int ry) {
		int px = (tiles.size() % (w/xu))*xu, py = (tiles.size() / (w/xu))*yu;
		for(int entity : recent_entities) {
			if(rx >= px && rx < px+xu && ry >= py && ry < py+yu) {
				return entity;
			}
			
			px += xu;
			if(px > x+w) {
				py += yu;
				px = x;
			}
		}
		return 0;
	}

private:
	std::pair<int, int> elem_position(char tile, int entity) {
		int px = x, py = y;

		for(char c : tiles) {
			if(c == tile) {
				return std::make_pair(px, py);
			}

			px += xu;
			if(px > x+w) {
				py += yu;
				px = x;
			}
		}
		
		for(int e : recent_entities) {
			if(e == entity) {
				return std::make_pair(px, py);
			}
		
			px += xu;
			if(px > x+w) {
				py += yu;
				px = x;
			}
		}
		
		return std::make_pair(-1, -1);
	}

public:
	void try_move(int dx, int dy) {
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

	char tile() {
		return selected;
	}

	int entity() {
		return entity_selected;	
	}

	void entity_select(int entity) {
		if(std::find(recent_entities.begin(), recent_entities.end(), entity) == recent_entities.end()) {
			recent_entities.push_back(entity);
			if(recent_entities.size() > MAX_RECENT_ENTITIES) {
				recent_entities.erase(recent_entities.begin());
			}
		}

		selected = 0;
		entity_selected = entity;
	}

	void select(char tile) {
		entity_selected = 0;
		selected = tile;
	}

	void unselect() {
		selected = 0;
		entity_selected = 0;
	}
};