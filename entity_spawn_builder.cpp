#include "entity_spawn_builder.h"
#include "known_entities.h"

#define SUBROUTINE_ALLOC 131072
#define FLOATS_ALLOC 16384

//+2
static BYTE arrow_trap_find[] = {0x80,0xCC,0xCC,0xCC,0xCC,0xCC,0x00,0xD9,0xCC,0xCC,0xCC,0xD9,0xCC,0xCC,0x89,0xCC,0xCC,0x74};
static std::string arrow_trap_mask = "x.....xx...x..x..x";

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

// AD (+1)  -> standard entity flag

static BYTE spawn_opcode[] = {
	0x6A, 0xAD,
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

//Special entities (idx < 100 || idx >= 2000)
//spawn_opcode push 0 instead of 1, and
//append the following opcodes after
// AA (+2) -> entity obj offset (1 byte)
// BB (+5) -> entity grid offset for specific entity (4 bytes)
static BYTE special_opcode[] = {
	0x8B,0x51,0xAA,
	0x89,0x82,0xBB,0xBB,0xBB,0xBB
};

//Flag append opcode (original use case for directional arrow traps)
//Normally used by appending after spawn_opcode (before special_opcode)
// AA (+2) -> entity flag offset (4 bytes)
// BB (+6) -> entity flag value (1 byte)
static BYTE flag_opcode[] = {
	0xC6, 0x80, 0xAA,0xAA,0xAA,0xAA, 0xBB
};

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

float EntitySpawnBuilder::EntitySpawn::x_pos() const {
	return x;
}

float EntitySpawnBuilder::EntitySpawn::y_pos() const {
	return y;
}

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

EntitySpawnBuilder::EntitySpawnBuilder(std::shared_ptr<GameHooks> gh) : 
	gh(gh),
	dp(gh->dp),
	spel(dp->spel),
	is_valid(true),
	spawn_entity_fn(0),
	floats_alloc(0),
	subroutine_alloc(0),
	unapplied_changes(true),
	arrow_trap_dir_offs(0)
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

	//find arrow trap dir offs
	arrow_trap_dir_offs = spel->get_stored_hook("arrow_trap_dir_offs");
	if(!arrow_trap_dir_offs) {
		Address cont = spel->find_mem(arrow_trap_find, arrow_trap_mask);
		if(!cont) {
			is_valid = false;
			DBG_EXPR(std::cout << "[EntitySpawnBuilder] Failed to find arrow_trap_dir_offs" << std::endl);
			return;
		}

		spel->read_mem(cont+2, &arrow_trap_dir_offs, sizeof(signed int));
		DBG_EXPR(std::cout << "[EntitySpawnBuilder] arrow_trap_dir_offs = " << arrow_trap_dir_offs << std::endl);
		spel->store_hook("arrow_trap_dir_offs", arrow_trap_dir_offs);
	}

	update_memory();
}

bool is_special_entity(int entity) {
	return (entity < 100 || entity >= 2000) && entity != 45;
}

static int strip_flags(int entity) {
	return entity & ~(W_TILE_BG_FLAG | ARROW_TRAP_LEFT_FACING);
}

void EntitySpawnBuilder::update_memory() {
	//if there are no new changes, don't update memory.	
	if(!unapplied_changes) {
		return;
	}

	Address flp = floats_alloc;
	
	Address game_ptr = dp->game_ptr();
	signed char entity_obj_offs = gh->entity_obj_offset();
	signed int entity_grid_base = gh->entity_grid_offset();
	signed int row_size = gh->entity_row_size();

	Address sr = subroutine_alloc;
	spel->write_mem(sr, init_opcode, sizeof(init_opcode));
	spel->write_mem(sr+3, &game_ptr, sizeof(Address));
	sr += sizeof(init_opcode);

	for(auto&& pair : entities) {
		auto& es = pair.second;

		int entity = strip_flags(es.entity);
		if(entity > 0) {
			bool special = is_special_entity(entity);
			char standard_val = special ? 0 : 1;

			spel->write_mem(flp, &es.x, sizeof(float));
			spel->write_mem(flp+4, &es.y, sizeof(float));
			
			signed int calldiff = spawn_entity_fn - (sr+29+5);
			Address xfl = flp, yfl = flp+4;
			spel->write_mem(sr, spawn_opcode, sizeof(spawn_opcode));
			spel->write_mem(sr+1, &standard_val, sizeof(char));
			spel->write_mem(sr+3, &entity, sizeof(int));
			spel->write_mem(sr+9, &yfl, sizeof(Address));
			spel->write_mem(sr+15, &xfl, sizeof(Address));
			spel->write_mem(sr+30, &calldiff, sizeof(signed int));
			spel->write_mem(sr+36, &game_ptr, sizeof(Address));

			sr += sizeof(spawn_opcode);

			if(es.entity & ARROW_TRAP_LEFT_FACING) {
				uint8_t one = 1;

				spel->write_mem(sr, flag_opcode, sizeof(flag_opcode));
				spel->write_mem(sr+2, &arrow_trap_dir_offs, sizeof(signed int));
				spel->write_mem(sr+6, &one, sizeof(uint8_t));

				sr += sizeof(flag_opcode);
			}

			if(special) {
				spel->write_mem(sr, special_opcode, sizeof(special_opcode));
				spel->write_mem(sr+2, &entity_obj_offs, sizeof(signed char));
				
				signed int grid_pos = entity_grid_base + sizeof(int)*(row_size*(int(99.0f - es.y) + 3) + int(es.x));
				spel->write_mem(sr+5, &grid_pos, sizeof(signed int));

				sr += sizeof(special_opcode);
			}

			flp += 8;
		}
	}

	spel->write_mem(sr, ret_opcode, sizeof(ret_opcode));
	unapplied_changes = false;
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
	unapplied_changes = true;
}

EntitySpawnBuilder::const_iterator EntitySpawnBuilder::erase(const_iterator pos) {
	auto ret = entities.erase(pos);
	unapplied_changes = true;
	return ret;
}

void EntitySpawnBuilder::clear() {
	entities.clear();
	unapplied_changes = true;
}