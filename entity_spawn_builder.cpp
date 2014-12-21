#include "entity_spawn_builder.h"

#define SUBROUTINE_ALLOC 16384
#define FLOATS_ALLOC 8192

//[SpawnEntity(void* current_game, float x, float y, int id, bool autoadd)]
//+0
static BYTE entity_spawn_find[] = {0xD9,0xCC,0x81,0xCC,0xCC,0xCC,0xCC,0xCC,0xD9,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x81};
static std::string entity_spawn_mask = "x.x.....x......x......x"; 

//in progress: 60 8B 0D B4 84 43 00 6A 01 68 0E 04 00 00 
//D9 05 04 00 0B 08 D9 05 00 00 0B 08 
//83 EC 08 
//D9 5C 24 04 
//D9 1C 24 
//E8 E7 FC 2F F5 61

//00 00 A0 40 00 00 A0 40

// AA (+3) -> game ptr
static BYTE init_opcode[] = {
	0x60, 
	0x8B,0x0D, 0xAA,0xAA,0xAA,0xAA
};

// AA (+3) -> entity id
// BB (+9) -> float:y
// CC (+15) -> float:x
// DD (+30) -> diff SpawnEntity (from +26)
// EE (+36) -> game ptr
//
// Currently not used:
// FF (+42) -> game ptr list offset
// AB (+45) -> list idx offset
// AC (+54) -> list idx offset
//
static BYTE spawn_opcode[] = {
	0x6A, 0x01,
	0x68, 0xAA,0xAA,0xAA,0xAA,
	0xD9,0x05, 0xBB,0xBB,0xBB,0xBB,
	0xD9,0x05, 0xCC,0xCC,0xCC,0xCC,
	0x83,0xEC,0x08,
	0xD9,0x1C,0x24,
	0xD9,0x5C,0x24,0x04,
	0xE8, 0xDD,0xDD,0xDD,0xDD,
	0x8B,0x0D, 0xEE,0xEE,0xEE,0xEE
};

static BYTE ret_opcode[] = {0x61, 0xC3};

EntitySpawnBuilder::EntitySpawn::EntitySpawn(float x, float y, int entity) : 
	x(x), 
	y(y), 
	entity(entity)
{}

EntitySpawnBuilder::EntitySpawn::EntitySpawn() : 
	x(0),
	y(0),
	entity(0)
{}

EntitySpawnBuilder::~EntitySpawnBuilder() {
	if(subroutine_alloc) {
		spel->free(subroutine_alloc);
		subroutine_alloc = 0;
	}

	if(floats_alloc) {
		spel->free(floats_alloc);
		floats_alloc = 0;
	}
}

EntitySpawnBuilder::EntitySpawnBuilder(std::shared_ptr<DerandomizePatch> dp) : 
	dp(dp),
	spel(dp->spel),
	is_valid(true),
	spawn_entity_fn(0),
	floats_alloc(0),
	subroutine_alloc(0)
{
	subroutine_alloc = spel->allocate(SUBROUTINE_ALLOC, true);
	if(!subroutine_alloc) {
		is_valid = false;
		DBG_EXPR(std::cout << "[EntitySpawnBuilder] Failed to allocate subroutine_alloc" << std::endl);
		return;
	}

	floats_alloc = spel->allocate(FLOATS_ALLOC, true);
	if(!floats_alloc) {
		is_valid = false;
		DBG_EXPR(std::cout << "[EntitySpawnBuilder] Failed to allocate floats_alloc" << std::endl);
		return;
	}

	DBG_EXPR(std::cout << "[EntitySpawnBuilder] subroutine_alloc = " << subroutine_alloc << ", floats_alloc = " << floats_alloc << std::endl);

	spawn_entity_fn = spel->get_stored_hook("spawn_entity_fn");
	if(!spawn_entity_fn) {
		spawn_entity_fn = spel->find_mem(entity_spawn_find, entity_spawn_mask);

		if(!spawn_entity_fn) {
			is_valid = false;
			DBG_EXPR(std::cout << "[EntitySpawnBuilder] Failed to find spawn_entity_fn" << std::endl);
			return;
		}

		DBG_EXPR(std::cout << "[EntitySpawnBuilder] Found spawn_entity_fn at " << spawn_entity_fn << std::endl);
		spel->store_hook("spawn_entity_fn", spawn_entity_fn);
	}

	update_memory();
}

void EntitySpawnBuilder::update_memory() {
	Address flp = floats_alloc;
	
	Address game_ptr = dp->game_ptr();

	Address sr = subroutine_alloc;
	spel->write_mem(sr, init_opcode, sizeof(init_opcode));
	spel->write_mem(sr+3, &game_ptr, sizeof(Address));
	sr += sizeof(init_opcode);

	for(auto&& pair : entities) {
		auto& es = pair.second;
		if(es.entity > 0) {
			spel->write_mem(flp, &es.x, sizeof(float));
			spel->write_mem(flp+4, &es.y, sizeof(float));
		
			signed int calldiff = spawn_entity_fn - (sr+29+5);
			Address xfl = flp, yfl = flp+4;
			spel->write_mem(sr, spawn_opcode, sizeof(spawn_opcode));
			spel->write_mem(sr+3, &es.entity, sizeof(int));
			spel->write_mem(sr+9, &yfl, sizeof(Address));
			spel->write_mem(sr+15, &xfl, sizeof(Address));
			spel->write_mem(sr+30, &calldiff, sizeof(signed int));
			spel->write_mem(sr+36, &game_ptr, sizeof(Address));

			sr += sizeof(spawn_opcode);
			flp += 8;
		}
	}

	spel->write_mem(sr, ret_opcode, sizeof(ret_opcode));
}

bool EntitySpawnBuilder::valid() const {
	return is_valid;
}

Address EntitySpawnBuilder::subroutine_addr() const {
	return subroutine_alloc;
}

EntitySpawnBuilder::const_iterator EntitySpawnBuilder::find(float x, float y) const {
	return entities.find(std::make_pair(x, y));
}

EntitySpawnBuilder::const_iterator EntitySpawnBuilder::begin() const {
	return entities.begin();
}

EntitySpawnBuilder::const_iterator EntitySpawnBuilder::end() const {
	return entities.end();
}

void EntitySpawnBuilder::add(float x, float y, int entity) {
	auto p = find(x, y);
	if(p != end()) {
		entities.erase(p);
	}
	
	entities[std::make_pair(x, y)] = EntitySpawn(x, y, entity);
}

EntitySpawnBuilder::const_iterator EntitySpawnBuilder::erase(const_iterator pos) {
	auto ret = entities.erase(pos);
	return ret;
}

void EntitySpawnBuilder::clear() {
	entities.clear();
}