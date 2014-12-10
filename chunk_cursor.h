#pragma once

#include <set>
#include <functional>
#include <vector>
#include <map>
#include "tile_chunk.h"

typedef std::map<std::pair<int, int>, char> cursor_store;

//works with constant-size chunks as a cursor
struct ChunkCursor {
private:
	typedef std::function<void(int dsx, int dsy, int dex, int dey)> pos_fn;

private:
	bool read_only;

	std::vector<pos_fn> pos_change_cb;

	std::vector<Chunk*> chunks;
	int tw;
	int width;
	int height;

	int sx;
	int sy;
	int ex;
	int ey;

public:
	char tile;
	
	ChunkCursor(const std::vector<Chunk*>& chunks, int tw, bool read_only=false);
	void pos_callback(pos_fn fn);
	
private:
	void tileref(int x, int y, std::function<void(Chunk* c, int cx, int cy)> ref);

	void write(char tile, int x, int y);
	char get(int x, int y) const;

public:
	char get() const;
	void put(char tile);
	
	bool in_bounds() const;

	bool try_dx(int dx);
	bool try_dy(int dy);
	bool try_dex(int dx);
	bool try_dey(int dy);
	bool try_dsx(int dx);
	bool try_dsy(int dy);

	int rsx() const;
	int rsy() const;
	int rex() const;
	int rey() const;

	void s(int x, int y);
	void e(int x, int y);
	
	int cc_width();
	int cc_height();

	cursor_store encode();

private:
	typedef std::set<std::pair<int, int>> fill_history;
	void fill_recurse(int x, int y, fill_history& history, char tile, char target);
public:
	void fill(char tile);


	void decode(const cursor_store& store);
};