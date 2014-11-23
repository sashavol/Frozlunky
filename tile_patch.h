#pragma once

#include "patches.h"
#include "tile_chunk.h"

#include <map>
#include <string>
#include <vector>

class TilePatch : public Patch {
private:
	bool is_valid;
	std::vector<Chunk*> chunks;
	std::map<std::string, SingleChunk*> chunk_origs;
	std::map<std::string, Address> chunk_addrs;
	std::map<std::string, Address> chunk_refs;
	std::map<std::string, Address> dyn_fn_addrs;

private:
	void scan_dyn_fn(const std::string& name);

public:
	TilePatch(std::shared_ptr<Spelunky> spel);
	~TilePatch();

private:
	virtual bool _perform() override;
	virtual bool _undo() override;

public:
	virtual bool valid() override;
	
	std::vector<Chunk*> query_chunks(const std::string& start);
	std::vector<Chunk*> all_chunks();

	void apply_tiles();
	
private:
	void revert_chunk(Chunk* cnk);
public:
	void revert_chunks();
};