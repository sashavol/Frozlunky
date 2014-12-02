#pragma once

#include <map>
#include <functional>
#include "tile_chunk.h"

typedef std::map<std::pair<int, int>, char> cursor_store;

//works with constant-size chunks as a cursor
struct ChunkCursor {
private:
	std::vector<Chunk*> chunks;
	int tw;
	int width;
	int height;

public:
	char tile;
	int sx;
	int sy;
	int ex;
	int ey;

	ChunkCursor(const std::vector<Chunk*>& chunks, int tw);

private:
	void tileref(int x, int y, std::function<void(Chunk* c, int cx, int cy)> ref);

	void write(char tile, int x, int y);
	char get(int x, int y) const;

public:
	void put(char tile);
	bool in_bounds();
	bool try_dx(int dx);
	bool try_dy(int dy);
	bool try_dex(int dx);
	bool try_dey(int dy);
	bool try_dsx(int dx);
	bool try_dsy(int dy);

	cursor_store encode();

	void decode(const cursor_store& store);
};