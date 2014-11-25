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

ChunkType SingleChunk::type() const {
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

GroupChunk::GroupChunk(const std::vector<Chunk*>& group, int w, int h) : 
	Chunk(group_name(group), w, h), 
	chunks(group) 
{}

void GroupChunk::tileref(int x, int y, std::function<void(Chunk*, int x)> fn) {
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
			fn(*it, x);
			break;
		}
	}
}

char GroupChunk::tile(int x, int y) const {
	char val = 0;
	(const_cast<GroupChunk* const>(this))->tileref(x, y, [&](Chunk* c, int rx) {
		val = c->tile(rx, 0);
	});
	return val;
}

void GroupChunk::tile(int x, int y, char tile) {
	tileref(x, y, [&](Chunk* c, int rx) {
		c->tile(rx, 0, tile);
	});
}

ChunkType GroupChunk::type() const {
	return ChunkType::Group;
}

std::vector<Chunk*> GroupChunk::get_chunks() {
	return chunks;
}

////////

//LinkedChunk::LinkedChunk(const std::vector<Chunk*>& chunks) : 
//	GroupChunk(chunks, chunks[0]->get_width(), chunks[0]->get_height()),
//	chunks(chunks)
//{
//	if(chunks.size() == 0)
//		throw std::runtime_error("No chunks passed to link.");
//
//	Chunk* base = chunks[0];
//	for(Chunk* c : chunks) {
//		if(c == base)
//			break;
//		
//		int w = base->get_width(), h = base->get_height();
//		for(int y = 0; y < h; y++) {
//			for(int x = 0; x < w; x++) {
//				c->tile(x, y, base->tile(x, y));
//			}
//		}
//	}
//}
//
//void LinkedChunk::tile(int x, int y, char val) {
//	for(Chunk* c : chunks) {
//		c->tile(x, y, val);
//	}
//}
//
//char LinkedChunk::tile(int x, int y) const {
//	return chunks[0]->tile(x, y);
//}