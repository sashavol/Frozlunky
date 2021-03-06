#pragma once

#include "tile_draw.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <set>
#include <map>

#define MAX_RECENT_ENTITIES 14

class TilePicker {
private:
	Fl_Widget* parent;
	int x, y, w, h, xu, yu, htiles;
	std::set<char> tiles;
	std::vector<int> recent_entities;
	char selected;
	int entity_selected;
	AreaRenderMode arm;

public:
	TilePicker(Fl_Widget* parent, AreaRenderMode arm, const std::set<char>& tiles, int x, int y, int w, int h, int xu, int yu);

	void draw();
	char tile(int rx, int ry);
	int entity(int rx, int ry);
	void resize(int x, int y, int w, int h, int xu, int yu);

private:
	std::pair<int, int> elem_position(char tile, int entity);

public:
	void try_move(int dx, int dy);
	
	char tile();
	int entity();

	std::vector<int>& get_recent_entities();
	std::set<char>& get_tiles();
	
	void entity_select(int entity);
	void select(char tile);
	void unselect();
};