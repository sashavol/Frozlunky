#include "entity_spawn_layer.h"
#include <boost/assign.hpp>
#include <set>
#include <map>

//some entities do not behave correctly if placed at the default tile location, so we special case..
static std::set<float> pos_yoffs;
static std::map<int, float> special_yoffs;
static std::map<std::pair<int, int>, float> special_yoffs_ranges = boost::assign::map_list_of
	(std::make_pair(100, 108), -0.3f)
	(std::make_pair(112, 114), -0.3f)
	(std::make_pair(118, 119), -0.3f)
	(std::make_pair(122, 122), -0.3f)
	(std::make_pair(124, 127), -0.3f)
	(std::make_pair(153, 154), -0.3f)
	(std::make_pair(210, 210), -0.3f)
	(std::make_pair(500, 533), -0.3f); //all items need shifting down

//ensures the possible tile y offsets are initialized
static void ensure_pos_yoffs_init() {
	if(pos_yoffs.size() == 0) {
		for(const std::pair<std::pair<int,int>,float>& range : special_yoffs_ranges) {
			for(int i = range.first.first; i <= range.first.second; ++i) {
				special_yoffs[i] = range.second;
			}
		
			if(pos_yoffs.find(range.second) == pos_yoffs.end()) {
				pos_yoffs.insert(range.second);
			}
		}

		pos_yoffs.insert(0.0f); //include default y offset, 0.0f (no change)
	}
}

EntitySpawnLayer::EntitySpawnLayer(std::shared_ptr<EntitySpawnBuilder> builder, int w, int h) : 
	builder(builder), 
	w(w), 
	h(h) 
{
	ensure_pos_yoffs_init();
}

EntitySpawnLayer::EntitySpawnLayer() : builder(nullptr), w(0), h(0) {}

std::pair<float, float> EntitySpawnLayer::map(int cx, int cy, int entity) {
	//special_yoffs[entity] defaults to 0.0f if entity is not special cased
	return std::make_pair(3.0f + (float)cx, (100.0f - (float)cy) - 1.0f + special_yoffs[entity]); 
}

std::pair<int, int> EntitySpawnLayer::unmap(float x, float y) {
	//conveniently special cased -1.0f < yoffs <= 0.0f are irrelevant when flooring
	return std::make_pair(int(x - 3.0f), int(99.0f - y));
}

EntitySpawnBuilder::const_iterator EntitySpawnLayer::entity_find(int x, int y) const {
	auto pos = map(x, y);
	
	//check all possible y offsets for entities
	for(float yoff : pos_yoffs) {
		auto f = builder->find(pos.first, pos.second + yoff);
		if(f != builder->end()) {
			return f;
		}
	}

	return builder->end();
}

void EntitySpawnLayer::put(int x, int y, int entity) {
	auto pos = map(x, y, entity);
	
	auto f = entity_find(x, y);
	if(f != builder->end()) {
		builder->erase(f);
	}
	
	if(entity > 0) {
		builder->add(pos.first, pos.second, entity);
	}
}

int EntitySpawnLayer::get(int x, int y) const {
	auto pos = map(x, y);
	
	auto f = entity_find(x, y);
	if(f != builder->end()) {
		return f->second.entity;
	}

	return 0;
}

int EntitySpawnLayer::get_w() const {
	return w;
}

int EntitySpawnLayer::get_h() const {
	return h;
}

EntitySpawnLayer::const_iterator EntitySpawnLayer::begin() const {
	return builder->begin();
}

EntitySpawnLayer::const_iterator EntitySpawnLayer::end() const {
	return builder->end();
}