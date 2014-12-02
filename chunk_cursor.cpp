#include "chunk_cursor.h"
#include "debug.h"
#include <iostream>

ChunkCursor::ChunkCursor(const std::vector<Chunk*>& chunks, int tw) : 
	tile('1'), 
	sx(-1), 
	sy(-1), 
	ex(-1), 
	ey(-1),
	chunks(chunks),
	tw(tw)
{
	if(chunks.empty()) {
		width = 0;
		height = 0;
		return;
	}

	width = chunks[0]->get_width()*tw;
	height = chunks.size()/tw * chunks[0]->get_height();
}

void ChunkCursor::tileref(int x, int y, std::function<void(Chunk* c, int cx, int cy)> ref) {
	int px = 0;
	int py = 0;
	int hx = 0;
	
	for(Chunk* c : chunks) {
		if(x >= px && x < px + c->get_width() && y >= py && y < py + c->get_height()) {
			ref(c, x - px, y - py);
		}

		px += c->get_width();
		hx++;
		if(hx >= tw) {
			py += c->get_height();
			px = 0;
			hx = 0;
		}
	}
}

void ChunkCursor::write(char tile, int x, int y) {
	tileref(x, y, [=](Chunk* c, int cx, int cy) {
		c->tile(cx, cy, tile);
	});
}

char ChunkCursor::get(int x, int y) const {
	char tile = 0;
	const_cast<ChunkCursor*>(this)->tileref(x, y, [&](Chunk* c, int cx, int cy) {
		tile = c->tile(cx, cy);
	});
	return tile;
}


void ChunkCursor::put(char tile) {
	if(!in_bounds()) {
		return;
	}

	try {
		for(int x = sx; x < ex; x++) {
			for(int y = sy; y < ey; y++) {
				write(tile, x, y);
			}
		}
	}
	catch(std::exception&) {
		DBG_EXPR(std::cout << "[TileEditorWidget] Warning: put('" << tile << "') out of bounds." << std::endl);
	}
}

bool ChunkCursor::in_bounds() {
	return sx >= 0 && ex <= width && sy >= 0 && ey <= height;
}

bool ChunkCursor::try_dx(int dx) {
	if(ex + dx > width || sx + dx < 0)
		return false;
	
	sx += dx;
	ex += dx;

	return true;
}

bool ChunkCursor::try_dy(int dy) {
	if(sy + dy < 0 || ey + dy > height)
		return false;
	
	sy += dy;
	ey += dy;

	return true;
}

cursor_store ChunkCursor::encode() {
	std::map<std::pair<int, int>, char> out;
	for(int x = sx; x < ex; x++) {
		for(int y = sy; y < ey; y++) {
			char tile = get(x, y);
			if(tile != 0) {
				out[std::pair<int, int>(x, y)] = tile;
			}
		}
	}
	return out;
}

void ChunkCursor::decode(const cursor_store& store) {
	if(!in_bounds()) {
		return;
	}

	std::pair<int, int> edge(INT_MAX, INT_MAX);
	for(auto&& p : store) {
		auto& at = p.first;
		edge.first = std::min(at.first, edge.first);
		edge.second = std::min(at.second, edge.second);
	}

	for(auto&& p : store) {
		auto at = p.first;
		write(p.second, at.first - edge.first + sx, at.second - edge.second + sy);
	}
}

bool ChunkCursor::try_dex(int dx) {
	int rex = ex + dx;
	if(rex < 0 || rex > width)
		return false;

	ex = rex;
	if(ex < sx)
		std::swap(sx, ex);
	
	return true;
}

bool ChunkCursor::try_dey(int dy) {
	int rey = ey + dy;
	if(rey < 0 || rey > height)
		return false;

	ey = rey;
	if(ey < sy)
		std::swap(sy, ey);

	return true;;
}

bool ChunkCursor::try_dsx(int dx) {
	int rsx = sx + dx;
	if(rsx < 0 || rsx > width)
		return false;

	sx = rsx;
	if(ex < sx)
		std::swap(sx, ex);

	return true;
}

bool ChunkCursor::try_dsy(int dy) {
	int rsy = sy + dy;
	if(rsy < 0 || rsy > height)
		return false;

	sy = rsy;
	if(ey < sy)
		std::swap(sy, ey);

	return true;
}