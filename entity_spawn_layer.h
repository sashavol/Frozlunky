#pragma once

#include "entity_spawn_builder.h"

//creates a 'grid-tile' abstraction from entity spawner list
class EntitySpawnLayer {
public:
	typedef EntitySpawnBuilder::const_iterator const_iterator;

private:
	std::shared_ptr<EntitySpawnBuilder> builder;
	int w;
	int h;

	EntitySpawnBuilder::const_iterator entity_find(int x, int y) const;

public:
	static std::pair<float, float> map(int cx, int cy, int entity=0);
	static std::pair<int, int> unmap(float x, float y);
	
	EntitySpawnLayer(std::shared_ptr<EntitySpawnBuilder> builder, int w, int h);
	EntitySpawnLayer();

	void put(int x, int y, int entity);
	int get(int x, int y) const;

	int get_w() const;
	int get_h() const;

	const_iterator begin() const;
	const_iterator end() const;
};