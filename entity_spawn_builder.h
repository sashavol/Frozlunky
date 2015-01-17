#pragma once

#include "patches.h"
#include "game_hooks.h"
#include <set>
#include <map>

bool is_special_entity(int entity);

class EntitySpawnBuilder {
public:
	struct EntitySpawn {
		int entity;
	
	private:
		float x;
		float y;
	
	public:
		EntitySpawn(float x, float y, int entity);
		EntitySpawn();

		float x_pos() const;
		float y_pos() const;

		friend class EntitySpawnBuilder;
	};

private:
	typedef std::map<std::pair<float, float>, EntitySpawn> holder_type;

public:
	typedef holder_type::const_iterator const_iterator;

private:
	std::shared_ptr<GameHooks> gh;
	std::shared_ptr<DerandomizePatch> dp; 
	std::shared_ptr<Spelunky> spel;
	bool is_valid;
	bool unapplied_changes;

	holder_type entities;

	Address spawn_entity_fn;
	Address subroutine_alloc;
	Address floats_alloc;

private:
	//prevent copy
	EntitySpawnBuilder(const EntitySpawnBuilder& o);

public:
	EntitySpawnBuilder(std::shared_ptr<GameHooks> gh);
	~EntitySpawnBuilder();

	void update_memory();

	bool valid() const;
	Address subroutine_addr() const;
	 
	void add(float x, float y, int entity);
	const_iterator erase(const_iterator pos);
	void clear();

	const_iterator find(float x, float y) const;
	const_iterator begin() const;
	const_iterator end() const;
};