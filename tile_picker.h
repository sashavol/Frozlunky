#pragma once

#include "tile_draw.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <set>

#define MAX_RECENT_ENTITIES 14

class TilePicker {
private:
	int x, y, w, h, xu, yu;
	std::set<char> tiles;
	std::vector<int> recent_entities;
	char selected;
	int entity_selected;
	AreaRenderMode arm;

public:
	TilePicker(AreaRenderMode arm, const std::set<char>& tiles, int x, int y, int w, int h, int xu, int yu);

	void draw();
	char tile(int rx, int ry);
	int entity(int rx, int ry);

private:
	std::pair<int, int> elem_position(char tile, int entity);

public:
	void try_move(int dx, int dy);
	
	char tile();
	int entity();

	const std::vector<int>& get_recent_entities();
	
	void entity_select(int entity);
	void select(char tile);
	void unselect();
};