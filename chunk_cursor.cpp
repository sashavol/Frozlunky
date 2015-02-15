#include "known_entities.h"
#include "chunk_cursor.h"
#include "debug.h"
#include <iostream>
#include <algorithm>

ChunkCursor::ChunkCursor(std::shared_ptr<GameHooks> gh, const std::vector<Chunk*>& chunks, std::shared_ptr<EntitySpawnBuilder> esb, int tw, bool read_only) :
	read_only(read_only),
	tile('1'), 
	sx(-1), 
	sy(-1), 
	ex(-1), 
	ey(-1),
	chunks(chunks),
	tw(tw),
	message_grid(gh)
{
	if(chunks.empty()) {
		width = 0;
		height = 0;
		return;
	}

	width = chunks[0]->get_width()*tw;
	height = chunks.size()/tw * chunks[0]->get_height();

	if(esb) {
		entity_layer = std::make_shared<EntitySpawnLayer>(esb, width, height);
	}

	sx = 0;
	sy = 0;
	ex = sx;
	ey = sy;
}

void ChunkCursor::pos_callback(pos_fn fn) {
	pos_change_cb.push_back(fn);
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

void ChunkCursor::write(char tile, int x, int y, bool zero_et_) {
	if(zero_et_) {
		erase_message(x, y);
		entity_write(0, x, y);
	}

	tileref(x, y, [=](Chunk* c, int cx, int cy) {
		c->tile(cx, cy, tile);
	});
}

void ChunkCursor::entity_write(int entity, int x, int y, bool zero_ti_) {
	if(!entity_layer) {
		return;
	}

	if(zero_ti_) {
		erase_message(x, y);

		if(entity & W_TILE_BG_FLAG)
			write('w', x, y, false);
		else
			write('0', x, y, false);
	}

	entity_layer->put(x, y, raw_entity(entity));
}

int ChunkCursor::entity_get(int x, int y) const {
	if(!entity_layer) {
		return 0;
	}

	int ret = entity_layer->get(x, y);
	
	if(ret != 0) {
		if(get(x, y) == 'w')
			ret |= W_TILE_BG_FLAG;
	}

	return ret;
}

char ChunkCursor::get(int x, int y) const {
	char tile = 0;
	const_cast<ChunkCursor*>(this)->tileref(x, y, [&](Chunk* c, int cx, int cy) {
		tile = c->tile(cx, cy);
	});
	return tile;
}

int ChunkCursor::entity_get() const {
	return entity_get(rsx(), rsy());
}

void ChunkCursor::entity_put(int entity) {
	if(!in_bounds() || read_only) {
		return;
	}

	int sx = rsx(), sy = rsy(), ex = rex(), ey = rey();

	try {
		for(int x = sx; x <= ex; x++) {
			for(int y = sy; y <= ey; y++) {
				entity_write(entity, x, y);
			}
		}
	}
	catch(std::exception&) {
		DBG_EXPR(std::cout << "[TileEditorWidget] Warning: entity_put('" << tile << "') out of bounds." << std::endl);
	}
}

char ChunkCursor::get() const {
	return get(rsx(), rsy());
}

void ChunkCursor::put(char tile) {
	if(!in_bounds() || read_only) {
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

std::string ChunkCursor::get_message() {
	auto it = message_grid.find(std::make_pair(rsx(), rsy()));
	if(it != message_grid.end())
		return it->second;
	else
		return "";
}

void ChunkCursor::put_message(const std::string& msg) {
	if(!in_bounds() || read_only) {
		return;
	}

	write_message(rsx(), rsy(), msg);
}

void ChunkCursor::write_message(int x, int y, const std::string& msg) {
	if(!entity_layer) {
		return;
	}
	
	entity_write(MESSAGE_ENTITY, x, y);
	message_grid.insert(std::make_pair(x, y), msg);
}

void ChunkCursor::erase_message(int x, int y) {
	entity_write(0, x, y, false);

	auto it = message_grid.find(std::make_pair(x, y));
	if(it != message_grid.end()) {
		message_grid.erase(it);
	}
}

void ChunkCursor::clear_messages() {
	message_grid.clear();
}

bool ChunkCursor::in_bounds() const {
	return sx >= 0 && sx < width 
		&& ex >= 0 && ex < width 
		&& sy >= 0 && sy < height 
		&& ey >= 0 && ey < height;
}

bool ChunkCursor::try_dx(int dx) {
	int osx = rsx(), oex = rex();

	if(ex + dx >= width || sx + dx < 0 || sx + dx >= width || ex + dx < 0)
		return false;
	
	sx += dx;
	ex += dx;

	for(auto&& cb : pos_change_cb) {
		cb(rsx() - osx, 0, rex() - oex, 0);
	}

	return true;
}

bool ChunkCursor::try_dy(int dy) {
	int osy = rsy(), oey = rey();

	if(sy + dy < 0 || ey + dy >= height || sy + dy >= height || ey + dy < 0)
		return false;
	
	sy += dy;
	ey += dy;

	for(auto&& cb : pos_change_cb) {
		cb(0, rsy() - osy, 0, rey() - oey);
	}

	return true;
}

void ChunkCursor::s(int x, int y) {
	int osx = rsx(), osy = rsy(), oex = rex(), oey = rey();

	int dx = x - sx, dy = y - sy;
	
	sx = x;
	sy = y;

	for(auto&& cb : pos_change_cb) {
		cb(rsx() - osx, rsy() - osy, rex() - oex, rey() - oey);
	}
}

void ChunkCursor::e(int x, int y) {
	int osx = rsx(), osy = rsy(), oex = rex(), oey = rey();

	ex = x;
	ey = y;

	for(auto&& cb : pos_change_cb) {
		cb(rsx() - osx, rsy() - osy, rex() - oex, rey() - oey);
	}
}

int ChunkCursor::rex() const {
	return max(sx, ex);
}

int ChunkCursor::rsx() const {
	return min(sx, ex);
}

int ChunkCursor::rey() const {
	return max(sy, ey);
}

int ChunkCursor::rsy() const {
	return min(sy, ey);
}


bool ChunkCursor::try_dex(int dx) {
	int osx = rsx(), oex = rex();

	int rex = ex + dx;
	if(rex < 0 || rex >= width)
		return false;

	ex = rex;
	
	for(auto&& cb : pos_change_cb) {
		cb(this->rsx() - osx, 0, this->rex() - oex, 0);
	}
	return true;
}

bool ChunkCursor::try_dey(int dy) {
	int osy = rsy(), oey = rey();

	int rey = ey + dy;
	if(rey < 0 || rey >= height)
		return false;

	ey = rey;
	
	for(auto&& cb : pos_change_cb) {
		cb(0, this->rsy() - osy, 0, this->rey() - oey);
	}
	
	return true;
}

bool ChunkCursor::try_dsx(int dx) {
	int osx = rsx(), oex = rex();

	int rsx = sx + dx;
	if(rsx < 0 || rsx >= width)
		return false;

	sx = rsx;
	
	for(auto&& cb : pos_change_cb) {
		cb(this->rsx() - osx, 0, this->rex() - oex, 0);
	}

	return true;
}

bool ChunkCursor::try_dsy(int dy) {
	int osy = rsy(), oey = rey();

	int rsy = sy + dy;
	if(rsy < 0 || rsy >= height)
		return false;

	sy = rsy;
	
	for(auto&& cb : pos_change_cb) {
		cb(0, this->rsy() - osy, 0, this->rey() - oey);
	}
	return true;
}

cursor_store ChunkCursor::encode() {
	int sx = rsx(), sy = rsy(), ex = rex(), ey = rey();
	
	cursor_store out;
	for(int x = sx; x <= ex; x++) {
		for(int y = sy; y <= ey; y++) {
			std::pair<int, int> pos = std::make_pair(x, y);

			char tile = get(x, y);
			if(tile != 0) {
				out.tiles[pos] = std::make_pair(tile, entity_get(x, y));
			}
		
			auto msg_it = message_grid.find(pos);
			if(msg_it != message_grid.end()) {
				out.messages[pos] = msg_it->second;
			}
		}
	}

	return out;
}

void ChunkCursor::decode(const cursor_store& store, bool horiz_mirror) {
	if(!in_bounds() || read_only) {
		return;
	}

	int sx = rsx(), sy = rsy(), ex = rex(), ey = rey();

	std::pair<int, int> edge(INT_MAX, INT_MAX);
	std::pair<int, int> end_edge(INT_MIN, INT_MIN);
	for(auto&& p : store.tiles) {
		auto& at = p.first;
		edge.first = min(at.first, edge.first);
		edge.second = min(at.second, edge.second);
		
		end_edge.first = max(at.first, end_edge.first);
		end_edge.second = max(at.second, end_edge.second);
	}

	if(entity_layer) {
		int width = end_edge.first - edge.first + 1;
		for(auto&& p : store.tiles) {
			std::pair<int, int> at = p.first;
			int x = (horiz_mirror ? width - (at.first - edge.first) - 1 : at.first - edge.first) + sx;
			int y = at.second - edge.second + sy;

			write(p.second.first, x, y);
			if(p.second.second != 0) {
				entity_write(p.second.second, x, y, false);
			}
		}

		for(auto&& m : store.messages) {
			MessageGrid::cc_pos at = m.first;
			int x = (horiz_mirror ? width - (at.first - edge.first) - 1 : at.first - edge.first) + sx;
			int y = at.second - edge.second + sy;

			message_grid.insert(std::make_pair(x, y), m.second);
		}
	}
}

bool ChunkCursor::check_dir(int x, int y, char target, int entity_target) {
	return entity_get(x, y) == entity_target && get(x, y) == target;
}

void ChunkCursor::fill_recurse(int x, int y, fill_history& history, char tile, int entity, char target, int entity_target) {
	if(history.find(std::make_pair(x-1,y)) == history.end() && x > 0 && check_dir(x-1, y, target, entity_target)) {
		history.insert(std::make_pair(x-1,y));
		fill_recurse(x-1, y, history, tile, entity, target, entity_target);
	}
	if(history.find(std::make_pair(x+1,y)) == history.end() && x < width - 1 && check_dir(x+1, y, target, entity_target)) {
		history.insert(std::make_pair(x+1,y));
		fill_recurse(x+1, y, history, tile, entity, target, entity_target);
	}
	if(history.find(std::make_pair(x,y-1)) == history.end() && y > 0 && check_dir(x, y-1, target, entity_target)) {
		history.insert(std::make_pair(x,y-1));
		fill_recurse(x, y-1, history, tile, entity, target, entity_target);
	}
	if(history.find(std::make_pair(x,y+1)) == history.end() && y < height - 1 && check_dir(x, y+1, target, entity_target)) {
		history.insert(std::make_pair(x,y+1));
		fill_recurse(x, y+1, history, tile, entity, target, entity_target);
	}

	if(entity)
		entity_write(entity, x, y);
	else if(tile)
		write(tile, x, y);
}

void ChunkCursor::fill(char tile) {
	if(read_only)
		return;

	fill_history history;
	fill_recurse(sx, sy, history, tile, 0, get(sx, sy), entity_get(sx, sy));
}

void ChunkCursor::entity_fill(int entity) {
	if(read_only)
		return;

	fill_history history;
	fill_recurse(sx, sy, history, '0', entity, get(sx, sy), entity_get(sx, sy));
}