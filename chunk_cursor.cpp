#include "chunk_cursor.h"
#include "debug.h"
#include <iostream>
#include <algorithm>

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

	sx = 0;
	sy = 0;
	ex = sx;
	ey = sy;
}

int ChunkCursor::cc_width() {
	return width;
}

int ChunkCursor::cc_height() {
	return height;
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

	int sx = rsx(), sy = rsy(), ex = rex(), ey = rey();

	try {
		for(int x = sx; x <= ex; x++) {
			for(int y = sy; y <= ey; y++) {
				write(tile, x, y);
			}
		}
	}
	catch(std::exception&) {
		DBG_EXPR(std::cout << "[TileEditorWidget] Warning: put('" << tile << "') out of bounds." << std::endl);
	}
}

bool ChunkCursor::in_bounds() {
	return sx >= 0 && sx < width 
		&& ex >= 0 && ex < width 
		&& sy >= 0 && sy < height 
		&& ey >= 0 && ey < height;
}

bool ChunkCursor::try_dx(int dx) {
	if(ex + dx >= width || sx + dx < 0 || sx + dx >= width || ex + dx < 0)
		return false;
	
	sx += dx;
	ex += dx;

	return true;
}

bool ChunkCursor::try_dy(int dy) {
	if(sy + dy < 0 || ey + dy >= height || sy + dy >= height || ey + dy < 0)
		return false;
	
	sy += dy;
	ey += dy;

	return true;
}

int ChunkCursor::rex() {
	return std::max(sx, ex);
}

int ChunkCursor::rsx() {
	return std::min(sx, ex);
}

int ChunkCursor::rey() {
	return std::max(sy, ey);
}

int ChunkCursor::rsy() {
	return std::min(sy, ey);
}

/*
static void trim_enc(cursor_store& enc, int h) {
	std::pair<int, int> f(INT_MAX, INT_MAX), s(-1, -1);

	for(auto& p : enc) {
		auto pos = p.first;
		if(p.second != '0') {
			f.first = std::min(pos.first, f.first);
			f.second = std::min(pos.second, f.second);
			s.first = std::max(pos.first, s.first);
			s.second = std::max(pos.second, s.second);
		}
	}
	
	for(auto i = enc.begin(); i != enc.end();) {
		auto pos = i->first;
		if((pos.first < f.first || pos.second < f.second) 
		|| (pos.first > s.first || pos.second > s.second))
		{
			i = enc.erase(i);
		}
		else
			++i;
	}
}
*/

cursor_store ChunkCursor::encode() {
	int sx = rsx(), sy = rsy(), ex = rex(), ey = rey();
	
	cursor_store out;
	for(int x = sx; x <= ex; x++) {
		for(int y = sy; y <= ey; y++) {
			char tile = get(x, y);
			if(tile != 0) {
				out[std::pair<int, int>(x, y)] = tile;
			}	
		}
	}

	//trim_enc(out, ey - sy);
	return out;
}

void ChunkCursor::decode(const cursor_store& store) {
	if(!in_bounds()) {
		return;
	}

	int sx = rsx(), sy = rsy(), ex = rex(), ey = rey();

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
	if(rex < 0 || rex >= width)
		return false;

	ex = rex;

	return true;
}

bool ChunkCursor::try_dey(int dy) {
	int rey = ey + dy;
	if(rey < 0 || rey >= height)
		return false;

	ey = rey;

	return true;
}

bool ChunkCursor::try_dsx(int dx) {
	int rsx = sx + dx;
	if(rsx < 0 || rsx >= width)
		return false;

	sx = rsx;

	return true;
}

bool ChunkCursor::try_dsy(int dy) {
	int rsy = sy + dy;
	if(rsy < 0 || rsy >= height)
		return false;

	sy = rsy;

	return true;
}

void ChunkCursor::fill_recurse(int x, int y, fill_history& history, char tile, char target) {
	if(history.find(std::pair<int,int>(x-1,y)) == history.end() && x > 0 && get(x-1, y) == target) {
		history.insert(std::pair<int,int>(x-1,y));
		fill_recurse(x-1, y, history, tile, target);
	}
	if(history.find(std::pair<int,int>(x+1,y)) == history.end() && x < width - 1 && get(x+1, y) == target) {
		history.insert(std::pair<int,int>(x+1,y));
		fill_recurse(x+1, y, history, tile, target);
	}
	if(history.find(std::pair<int,int>(x,y-1)) == history.end() && y > 0 && get(x, y-1) == target) {
		history.insert(std::pair<int,int>(x,y-1));
		fill_recurse(x, y-1, history, tile, target);
	}
	if(history.find(std::pair<int,int>(x,y+1)) == history.end() && y < height - 1 && get(x, y+1) == target) {
		history.insert(std::pair<int,int>(x,y+1));
		fill_recurse(x, y+1, history, tile, target);
	}

	write(tile, x, y);
}

void ChunkCursor::fill(char tile) {
	fill_history history;
	fill_recurse(sx, sy, history, tile, get(sx, sy));
}