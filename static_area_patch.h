#pragma once

#include "tile_chunk.h"
#include "patches.h"
#include "derandom.h"
#include "entity_spawn_builder.h"

#define CHUNK_LEN 81
#define CHUNK_WIDTH 10
#define CHUNK_HEIGHT 8

class StaticAreaPatch : public Patch {
private:
	std::string name;
	std::shared_ptr<DerandomizePatch> dp;
	std::shared_ptr<GameHooks> gh;
	std::map<int, std::shared_ptr<EntitySpawnBuilder>> builders;
	std::map<int, std::shared_ptr<bool>> dark_levels;
	std::map<int, Address> dark_level_valaddrs;

	Address gen_fn;
	Address gen_fn_end;
	
	int lvl_chunks;
	int lvl_start;
	int lvl_end;

	//standard chunk name:
	//[BaseArea](-[Lvl#])-[Seg#]
	std::map<std::string, Address> named_allocs;
	std::map<int, Address> allocs;
	std::map<Chunk*, int> level_parents;
	std::vector<SingleChunk*>* chunks;
	Address chunk_alloc;
	Address subroutine_alloc;

	BYTE eaxmov_opcode;

	Address insert_addr;
	BYTE* insert_orig;
	size_t insert_size;

	Address jmpout_addr;
	BYTE* jmpout_pre;
	size_t jmpout_pre_size;

	bool is_valid;

private:
	bool _perform() override;
	bool _undo() override;

private:
	bool find_jmp(); //finds where to jump after pushing chunk
	std::pair<Address, size_t> find_insert(); //finds where to insert subroutine
	BYTE find_cidx_register(); //finds what register the chunk index is stored at
								//and returns the corresponding byte for a mov eax, ... operation.
								// returns 0 on failure

public:
	//lvl_start -> start level # (inclusive)
	//lvl_end   -> end level # (exclusive)
	//if lvl_start == -1, single chunk applied statically across all levels
	~StaticAreaPatch();
	StaticAreaPatch(const std::string& name, std::shared_ptr<GameHooks> gh, Address gen_fn, int lvl_start, int lvl_end, bool single_level=false, int level_chunks=16);
	
	bool valid() override;

public:
	void apply_chunks();
	std::vector<Chunk*> query_chunks(const std::string& start);
	std::vector<Chunk*> get_chunks();
	std::vector<SingleChunk*> root_chunks();
	std::shared_ptr<EntitySpawnBuilder> entity_builder(int lvl);
	std::shared_ptr<bool> dark_status(int lvl);

	const std::string& get_name();
	int level_start();
	int level_end();

	//attempts to identify this chunk, if this patch does not own the chunk returns -1, otherwise the level number the chunk belongs to
	int identify_chunk(Chunk* cnk);
};