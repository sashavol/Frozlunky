#pragma once

#include "patches.h"
#include "tile_chunk.h"
#include "tile_patch.h"
#include "seeder.h"
#include "static_area_patch.h"
#include "derandom.h"

class StaticChunkPatch : public Patch {
private:
	std::shared_ptr<DerandomizePatch> dp;
	std::shared_ptr<Seeder> seeder;
	std::shared_ptr<TilePatch> tp;
	std::vector<std::shared_ptr<StaticAreaPatch>> saps;
	
	bool is_valid;

public:
	StaticChunkPatch(std::shared_ptr<DerandomizePatch> spel, std::shared_ptr<TilePatch> tp, std::shared_ptr<Seeder> seeder);
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
	
	void apply_chunks();
	bool valid_tile(char tile);
};