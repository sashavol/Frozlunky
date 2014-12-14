#include "tile_patch.h"
#include <iostream>
#include <algorithm>

static char extension_tiles[] = {'H', 'Z'};

static BYTE LevelGen_Selector_find[] = {0x81,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x8B};
static std::string LevelGen_Selector_mask = "x......x.......x......x";

static BYTE LevelGen_HellCnk_find[] = {0x83,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0x8B,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x83};
static std::string LevelGen_HellCnk_mask = "x....x...x..x.x......x";

static BYTE LevelGen_IceCavesGeneralCnk_find[] = {0x83,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x83};
static std::string LevelGen_IceCavesGeneralCnk_mask = "x...x......x.x......x";

static BYTE LevelGen_IceCavesSpaceshipCnk_find[] = {0x83,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0x83,0xCC,0xCC,0x83,0xCC,0xCC,0x8B,0xCC,0x8B};
static std::string LevelGen_IceCavesSpaceshipCnk_mask = "x....x....x..x..x.x";

static BYTE LevelGen_IceCavesYetiCnk_find[] = {0x83,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x83};
static std::string LevelGen_IceCavesYetiCnk_mask = "x....x....x.x......x";

static BYTE LevelGen_JungleBlackMarketCnk_find[] = {0x83,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x83,0xCC,0xCC,0x83,0xCC,0xCC,0x7C};
static std::string LevelGen_JungleBlackMarketCnk_mask = "x....x.x......x..x..x";

static BYTE LevelGen_JungleGeneralCnk_find[] = {0x83,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x83};
static std::string LevelGen_JungleGeneralCnk_mask = "x..x.......x.x......x";

static BYTE LevelGen_JungleHauntedMansionCnk_find[] = {0x83,0xCC,0xCC,0xCC,0xCC,0x83,0xCC,0xCC,0x83,0xCC,0xCC,0x8B,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x7C};
static std::string LevelGen_JungleHauntedMansionCnk_mask = "x....x..x..x.x......x";

static BYTE LevelGen_MinesCnk_find[] = {0x83,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x83,0xCC,0xCC,0x83};
static std::string LevelGen_MinesCnk_mask = "x...x....x.x......x..x";

static BYTE LevelGen_OlmecCnk_find[] = {0x83,0xCC,0xCC,0xCC,0x33,0xCC,0x83,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x75,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0x38};
static std::string LevelGen_OlmecCnk_mask = "x...x.x........x.x.....x";

static BYTE LevelGen_TempleCnk_find[] = {0x83,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0x8B,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x83,0xCC,0xCC,0x83};
static std::string LevelGen_TempleCnk_mask = "x....x.x.x......x..x";

static BYTE LevelGen_TutorialCnk_find[] = {0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0x83,0xCC,0xCC,0xCC,0x81};
static std::string LevelGen_TutorialCnk_mask = "x......x.....x.....x...x";

static BYTE LevelGen_WormCnk_find[] = {0x83,0xCC,0xCC,0xCC,0x8B,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x83,0xCC,0xCC,0x83,0xCC,0xCC,0x7C};
static std::string LevelGen_WormCnk_mask = "x...x.x.......x..x..x";


static std::pair<int, int> wh_determine(unsigned stlen) {
	switch(stlen) {
	case 80:
		return std::pair<int, int>(10, 8); // room template
	case 5:
		return std::pair<int, int>(5, 1); // obstacle, ideally represented as a group
	case 3:
		return std::pair<int, int>(3, 1);
	default:
		return std::pair<int, int>(-1, -1);
	}
}

const std::set<char>& TilePatch::possible_tiles() const {
	return pos_tiles;
}

bool TilePatch::valid_tile(char tile) const {
	return pos_tiles.find(tile) != pos_tiles.end();
}

//TODO some obstacles are still grouped as 1-height
void TilePatch::scan_dyn_fn(const std::string& name) {
	static BYTE end_find[] = {0x83, 0xC4, 0xCC, 0xC3};
	static BYTE end2_find[] = {0x83, 0xC4, 0xCC, 0xC2};
	static std::string end_mask = "xx.x";

	static BYTE strpush_find[] = {0x68, 0xCC, 0xCC, 0xCC, 0xCC};
	static std::string strpush_mask = "x....";

	static BYTE nil_find[] = {0x00};
	static std::string nil_mask = "x";

	Address start_addr = dyn_fn_addrs[name];
	Address end;
	
	//determine stop scanning address
	{
		Address end1 = spel->find_mem(end_find, end_mask, start_addr);
		Address end2 = spel->find_mem(end2_find, end_mask, start_addr);
		if(end1 == 0 && end2 != 0)
			end = end2;
		else if(end2 == 0 && end1 != 0)
			end = end1;
		else
			end = min(end1, end2);
	
		if(end == 0) {
			DBG_EXPR(std::cout << "[TilePatch] Failed to find return for " << name << std::endl);
			is_valid = false;
			return;
		}
		
		DBG_EXPR(std::cout << "[TilePatch] " << name << " will be scanned up to " << end << std::endl);
	}


	unsigned idx = 0;
	std::vector<SingleChunk*> single_chunks;

	for(Address p = spel->find_mem(strpush_find, strpush_mask, start_addr); !!p && p < end; p = spel->find_mem(strpush_find, strpush_mask, p+1)) 
	{
		Address str_ptr = 0;
		spel->read_mem(p+1, &str_ptr, sizeof(Address));
		
		if(!str_ptr) {
			//DBG_EXPR(std::cout << "[TilePatch] Warning: Found invalid pointer, or failed to read pointer for cnk " << chunk_name << std::endl);
			continue;
		}
		
		Address str_ptr_end = spel->find_mem(nil_find, nil_mask, str_ptr);
		if(!str_ptr_end) {
			//DBG_EXPR(std::cout << "[TilePatch] Warning: Failed to find end of string for string at " << str_ptr << " for cnk " << chunk_name << std::endl);
			continue;
		}

		auto strm_size = str_ptr_end - str_ptr + 1;
		char* str = new char[strm_size];
		spel->read_mem(str_ptr, str, strm_size);

		//chunk name is computed from the base area function name + _ + hash of the chunk string
		std::string chunk_name = name + "_" + std::to_string(std::hash<std::string>()(std::string(str))) + "-" + std::to_string(idx);

		auto wh = wh_determine(strm_size - 1);
		if(wh.first >= 0 && wh.second >= 0) {
			single_chunks.push_back(new SingleChunk(chunk_name, std::string(str), wh.first, wh.second));
			chunk_origs[chunk_name] = new SingleChunk(chunk_name, std::string(str), wh.first, wh.second);
			chunk_addrs[chunk_name] = str_ptr;
			chunk_refs[chunk_name] = p;
		}
		else {
			DBG_EXPR(std::cout << "[TilePatch] Warning: Encountered unknown chunk length: " << strm_size - 1 << " :: '" << str << "'" << std::endl);
		}

		idx++;
	}

	//record tile types
	for(SingleChunk* sc : single_chunks) {
		const std::string& data = sc->get_data();
		for(char c : data) {
			if(pos_tiles.find(c) == pos_tiles.end())
				pos_tiles.insert(c);
		}
	}

	int single_count = 0, group_count = 0;
	for(auto it = single_chunks.begin(); it != single_chunks.end(); ++it) {
		if((*it)->get_height() == 1) {
			std::vector<Chunk*> group;
			group.push_back(*it);

			//check how close two string references are, if they are closer than 0x15 bytes assume they are grouped
			int diff;
			while((it+1) != single_chunks.end() 
				&& (diff = (chunk_refs[(*(it+1))->get_name()] - chunk_refs[(*it)->get_name()])) > 0xA 
				&& diff < 0x15)
			{
				group.push_back(*(++it));
			}
			
			if(group.size() > 1) {
				chunks.push_back(new GroupChunk(group));
				group_count++;
			}
			else {
				DBG_EXPR(std::cout << "Warning: Encountered 1-height single chunk: " << chunk_refs[(*it)->get_name()] << " " << (*it)->get_data() << std::endl;);
				chunks.push_back(*it);
				single_count++;
			}
		}
		else {
			chunks.push_back(*it);
			single_count++;
		}
	}

	DBG_EXPR(std::cout << "[TilePatch] Found " << single_chunks.size() << " total chunks: " << single_count << " single, " << group_count  << " grouped" << std::endl);
}

Address TilePatch::rel_chunk_ref(const Chunk* cnk) {
	if(cnk->type() == ChunkType::Single) {
		return chunk_refs[static_cast<const SingleChunk*>(cnk)->get_name()];
	}
	else {
		return chunk_refs[const_cast<GroupChunk*>(static_cast<const GroupChunk*>(cnk))->get_chunks()[0]->get_name()];
	}
}

static void delete_chunk(Chunk* cnk) {
	if(cnk->type() == ChunkType::Single)
		delete static_cast<SingleChunk*>(cnk);
	else {
		GroupChunk* gc = static_cast<GroupChunk*>(cnk);
		for(Chunk* c : gc->get_chunks()) {
			delete_chunk(c);
		}
	}
}

TilePatch::~TilePatch() {
	for(Chunk* c : chunks) {
		delete_chunk(c);
	}
	for(std::pair<const std::string, SingleChunk*>& sc : chunk_origs) {
		delete_chunk(sc.second);
	}

	chunks.clear();
	chunk_origs.clear();
}


#define DISCOVER_CNK(name) { \
	if(!is_valid) { \
		return; \
	} \
	Address val = spel->find_mem(name##_find, name##_mask); \
	if(!val) { \
		DBG_EXPR(std::cout << "[TilePatch] Failed to find chunk generator for " #name << std::endl); \
		is_valid = false; \
		return; \
	} \
	DBG_EXPR(std::cout << "[TilePatch] Found " #name " at " << val << std::endl); \
	dyn_fn_addrs[#name] = val; \
	scan_dyn_fn(#name); \
}


TilePatch::TilePatch(std::shared_ptr<Spelunky> spel) : 
	Patch(spel), 
	is_valid(true) 
{
	DISCOVER_CNK(LevelGen_HellCnk);
	DISCOVER_CNK(LevelGen_IceCavesGeneralCnk);
	DISCOVER_CNK(LevelGen_IceCavesSpaceshipCnk);
	DISCOVER_CNK(LevelGen_IceCavesYetiCnk);
	DISCOVER_CNK(LevelGen_JungleBlackMarketCnk);
	DISCOVER_CNK(LevelGen_JungleGeneralCnk);
	DISCOVER_CNK(LevelGen_JungleHauntedMansionCnk);
	DISCOVER_CNK(LevelGen_MinesCnk);
	DISCOVER_CNK(LevelGen_OlmecCnk);
	DISCOVER_CNK(LevelGen_TempleCnk);
	DISCOVER_CNK(LevelGen_TutorialCnk);
	DISCOVER_CNK(LevelGen_WormCnk);
	
	//extensions to possible tiles
	for(char ext : extension_tiles) {
		if(pos_tiles.find(ext) == pos_tiles.end())
			pos_tiles.insert(ext);
	}

	DBG_EXPR(
		std::cout << "Possible tiles: ";
		for(char c : pos_tiles) {
			std::cout << c;
		}
		std::cout << std::endl;
	);
}

Address TilePatch::get_gen_fn(const std::string& name) {
	return dyn_fn_addrs[name];
}

bool TilePatch::_perform() {
	return true;
}

bool TilePatch::_undo() {
	for(auto&& c : chunk_origs) {
		const std::string& data = c.second->get_data();
		spel->write_mem(chunk_addrs[c.first], data.c_str(), data.size()+1, true);
	}
	return true;
}


bool TilePatch::valid() {
	return is_valid;
}


void TilePatch::apply_chunk(Chunk* cnk) {
	if(cnk->type() == ChunkType::Single) {
		SingleChunk* sc = static_cast<SingleChunk*>(cnk);

		const std::string& data = sc->get_data();
		spel->write_mem(chunk_addrs[sc->get_name()], data.c_str(), data.size()+1, true);
	}
	else {
		for(Chunk* c : static_cast<GroupChunk*>(cnk)->get_chunks()) {
			apply_chunk(c);
		}
	}
}

void TilePatch::apply_chunks() {
	if(this->is_active()) {
		for(Chunk* c : chunks) {
			apply_chunk(c);
		}
	}
}

void TilePatch::revert_chunk(Chunk* cnk) {
	 if(cnk->type() == ChunkType::Single) {
		 SingleChunk* sc = static_cast<SingleChunk*>(cnk);
		 sc->set_data(chunk_origs[sc->get_name()]->get_data());
	 }
	 
	 for(Chunk* c : static_cast<GroupChunk*>(cnk)->get_chunks()) {
		 revert_chunk(c);
	 }
}

void TilePatch::revert_chunks() {
	for(Chunk* c : chunks) {
		revert_chunk(c);
	}
}

std::vector<Chunk*> TilePatch::all_chunks() {
	return chunks;
}

std::vector<Chunk*> TilePatch::query_chunks(const std::string& start) {
	std::vector<Chunk*> out;
	for(Chunk* c : chunks) {
		if(c->get_name().find(start) == 0) {
			out.push_back(c);
		}
	}
	return out;
}


static void root_cnk(Chunk* c, std::vector<SingleChunk*>& sc) {
	if(c->type() == ChunkType::Single) {
		sc.push_back(static_cast<SingleChunk*>(c));
	}
	else {
		for(Chunk* ck : static_cast<GroupChunk*>(c)->get_chunks()) {
			root_cnk(ck, sc);
		}
	}
}

std::vector<SingleChunk*> TilePatch::root_chunks() {
	std::vector<SingleChunk*> out;
	for(Chunk* c : chunks) {
		root_cnk(c, out);
	}

	std::sort(out.begin(), out.end(), [=](const SingleChunk* a, const SingleChunk* b) {
		return chunk_refs[a->get_name()] < chunk_refs[b->get_name()];
	});

	return out;
}