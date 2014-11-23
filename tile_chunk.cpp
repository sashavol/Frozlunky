#include "tile_chunk.h"
#include <stdexcept>
#include <algorithm>
#include <sstream>

Chunk::Chunk(const std::string& name, int w, int h) : 
	width(w), height(h), name(name)
{}

std::string Chunk::get_name() const {
	return name;
}

int Chunk::get_width() const {
	return width;
}

int Chunk::get_height() const {
	return height;
}

//////////////

SingleChunk::SingleChunk(const std::string& name, const std::string& data, int w, int h) : 
	Chunk(name, w, h),
	data(data)
{}

char SingleChunk::tile(int x, int y) const {
	return data[y*get_width() + x];
}

void SingleChunk::tile(int x, int y, char tile) {
	data[y*get_width() + x] = tile;
}

ChunkType SingleChunk::type() {
	return ChunkType::Single;
}

const std::string& SingleChunk::get_data() const {
	return data;
}

void SingleChunk::set_data(const std::string& nd) {
	if(this->data.size() != nd.size())
		throw std::runtime_error("Mismatched new data size.");

	this->data = nd;
}

////////////

static int group_height(const std::vector<Chunk*>& group) {
	int h = 0;
	for(Chunk* c : group) {
		h += c->get_height();
	}
	return h;
}

//computed as a hash of the sum of the names of the chunks in the group sorted lexicographically by name
static std::string group_name(std::vector<Chunk*> group) {
	std::sort(group.begin(), group.end(), [](const Chunk* a, const Chunk* b) {
		return a->get_name() < b->get_name();
	});

	std::ostringstream oss;
	for(auto&& cnk : group) {
		oss << cnk->get_name();
	}

	return group[0]->get_name() + "-" + std::to_string(std::hash<std::string>()(oss.str()));
}

GroupChunk::GroupChunk(const std::vector<Chunk*>& group) : 
	Chunk(group_name(group), group[0]->get_width(), group_height(group)),
	chunks(group)
{}


void GroupChunk::tileref(int x, int y, std::function<void(Chunk*, int x, int y)> fn) {
	if(y >= get_height() || x >= get_width())
		throw std::runtime_error("Out of bounds tile.");
	
	auto it = chunks.begin();
	while(y > 0) {
		int ch = (*it)->get_height();
		if(ch >= y) {
			y -= ch;
			++it;
		}
		else {
			fn(*it, x, y);
			break;
		}
	}
}

char GroupChunk::tile(int x, int y) const {
	char val = 0;
	(const_cast<GroupChunk* const>(this))->tileref(x, y, [&](Chunk* c, int rx, int ry) {
		val = c->tile(rx, ry);
	});
	return val;
}

void GroupChunk::tile(int x, int y, char tile) {
	tileref(x, y, [&](Chunk* c, int rx, int ry) {
		c->tile(rx, ry, tile);
	});
}

ChunkType GroupChunk::type() {
	return ChunkType::Group;
}

std::vector<Chunk*> GroupChunk::get_chunks() {
	return chunks;
}