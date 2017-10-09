#pragma once

#include "patches.h"
#include "tile_chunk.h"
#include "tile_patch.h"
#include "seeder.h"
#include "static_area_patch.h"
#include "derandom.h"
#include "yeti_remove_patch.h"

class StaticChunkPatch : public Patch {
public:
	std::shared_ptr<GameHooks> gh;
	std::shared_ptr<DerandomizePatch> dp;

private:
	std::shared_ptr<Seeder> seeder;
	std::shared_ptr<TilePatch> tp;
	std::shared_ptr<YetiRemovePatch> yrp;
	std::vector<std::shared_ptr<StaticAreaPatch>> saps;
	
	Address jngl_anticrash_addr;
	BYTE* jngl_anticrash_orig;

	bool is_valid;

public:
	StaticChunkPatch(std::shared_ptr<GameHooks> gh, std::shared_ptr<TilePatch> tp, std::shared_ptr<Seeder> seeder);
	~StaticChunkPatch();

	virtual bool _perform() override;
	virtual bool _undo() override;
	
	virtual bool valid() override;

public:
	//returns 16 chunks in generation order
	std::vector<Chunk*> query_chunks(const std::string& name);
	std::vector<SingleChunk*> root_chunks();

	//returns all chunks.
	std::vector<Chunk*> get_chunks();

	const std::shared_ptr<TilePatch> tile_patch() const;
	
	void apply_chunks();
	bool valid_tile(char tile);
	const std::set<char>& valid_tiles();

	std::pair<std::shared_ptr<StaticAreaPatch>, int> parent(Chunk* cnk);
	std::shared_ptr<StaticAreaPatch> area_patch(const std::string& name);
};