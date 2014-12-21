#include "entity_spawn_layer.h"

EntitySpawnLayer::EntitySpawnLayer(std::shared_ptr<EntitySpawnBuilder> builder, int w, int h) : 
	builder(builder), 
	w(w), 
	h(h) 
{}

EntitySpawnLayer::EntitySpawnLayer() : builder(nullptr), w(0), h(0) {}

std::pair<float, float> EntitySpawnLayer::map(int cx, int cy) {
	return std::pair<float, float>(3.0f + (float)cx, (100.0f - (float)cy) - 1.0f); 
}

std::pair<int, int> EntitySpawnLayer::unmap(float x, float y) {
	return std::pair<int, int>(int(x - 3.0f), int(99.0f - y));	
}


void EntitySpawnLayer::put(int x, int y, int entity) {
	auto pos = map(x, y);
	
	auto f = builder->find(pos.first, pos.second);
	if(f != builder->end()) {
		builder->erase(f);
	}
	
	if(entity > 0) {
		builder->add(pos.first, pos.second, entity);
	}
}

int EntitySpawnLayer::get(int x, int y) const {
	auto pos = map(x, y);

	auto f = builder->find(pos.first, pos.second);
	if(f == builder->end()) {
		return 0;
	}
	else {
		return f->second.entity;
	}
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