#pragma once

#include "patches.h"
#include "tile_chunk.h"

#include <map>
#include <string>
#include <set>

#define MAX_LINK_SIZE 3

class TilePatch : public Patch {
private:
	bool is_valid;
	std::vector<Chunk*> chunks;
	std::map<std::string, SingleChunk*> chunk_origs;
	std::map<std::string, Address> chunk_addrs;
	std::map<std::string, Address> chunk_refs;
	std::map<std::string, Address> dyn_fn_addrs;
	std::set<char> pos_tiles;

private:
	void scan_dyn_fn(const std::string& name);
	//void link_chunks();
	Address rel_chunk_ref(const Chunk* cnk);

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
	std::vector<SingleChunk*> root_chunks();

	const std::set<char>& possible_tiles() const;
	bool valid_tile(char tile) const;

private:
	void apply_chunk(Chunk* cnk);
public:
	void apply_chunks();
	
private:
	void revert_chunk(Chunk* cnk);
public:
	void revert_chunks();
};