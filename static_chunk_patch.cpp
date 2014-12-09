#include "static_chunk_patch.h"

//+6
static BYTE jngl_anticrash_find[] = {0x81,0xCC,0xCC,0xCC,0xCC,0xCC,0x7F,0xCC,0x85,0xCC,0x74,0xCC,0x39,0xCC,0xCC,0x74,0xCC,0xD8,0xCC,0xCC,0xDF};
static std::string jngl_anticrash_mask = "x.....x.x.x.x..x.x..x";

//force jmp away from rushing water levels
static BYTE jngl_anticrash_mod[] = {0xEB}; 

StaticChunkPatch::~StaticChunkPatch() {
	if(jngl_anticrash_orig) {
		delete[] jngl_anticrash_orig;
		jngl_anticrash_orig = nullptr;
	}
}

StaticChunkPatch::StaticChunkPatch(std::shared_ptr<DerandomizePatch> dp, std::shared_ptr<TilePatch> tp, std::shared_ptr<Seeder> seeder) : 
	Patch(dp->spel),
	seeder(seeder),
	tp(tp),
	is_valid(true),
	jngl_anticrash_orig(nullptr),
	jngl_anticrash_addr(0)
{
	yrp = std::make_shared<YetiRemovePatch>(dp->spel);
	if(!yrp->valid()) {
		is_valid = false;
		DBG_EXPR(std::cout << "[StaticChunkPatch] Dependency on Yeti remove patch unsatisfied." << std::endl);
		return;
	}

	jngl_anticrash_addr = spel->find_mem(jngl_anticrash_find, jngl_anticrash_mask);
	if(!jngl_anticrash_addr) {
		is_valid = false;
		DBG_EXPR(std::cout << "[StaticChunkPatch] Failed to find jungle anticrash address." << std::endl);
		return;
	}
	jngl_anticrash_addr += 6;
	DBG_EXPR(std::cout << "[StaticChunkPatch] Writing anticrash patch at " << jngl_anticrash_addr << std::endl);

	jngl_anticrash_orig = new BYTE[sizeof(jngl_anticrash_mod)];
	spel->read_mem(jngl_anticrash_addr, jngl_anticrash_orig, sizeof(jngl_anticrash_mod));

	saps.push_back(std::shared_ptr<StaticAreaPatch>(new StaticAreaPatch("Tutorial", dp, tp->get_gen_fn("LevelGen_TutorialCnk"), 1, 3+1)));
	saps.push_back(std::shared_ptr<StaticAreaPatch>(new StaticAreaPatch("Mines", dp, tp->get_gen_fn("LevelGen_MinesCnk"), 1, 4+1)));
	saps.push_back(std::shared_ptr<StaticAreaPatch>(new StaticAreaPatch("Jungle", dp, tp->get_gen_fn("LevelGen_JungleGeneralCnk"), 5, 8+1)));
	saps.push_back(std::shared_ptr<StaticAreaPatch>(new StaticAreaPatch("JungleBlackMarket", dp, tp->get_gen_fn("LevelGen_JungleBlackMarketCnk"), 5, 8+1, true)));
	saps.push_back(std::shared_ptr<StaticAreaPatch>(new StaticAreaPatch("JungleHauntedCastle", dp, tp->get_gen_fn("LevelGen_JungleHauntedMansionCnk"), 5, 8+1, true)));
	saps.push_back(std::shared_ptr<StaticAreaPatch>(new StaticAreaPatch("IceCaves", dp, tp->get_gen_fn("LevelGen_IceCavesGeneralCnk"), 9, 12+1)));
	//OPT add support for mothership
	//saps.push_back(std::shared_ptr<StaticAreaPatch>(new StaticAreaPatch("IceCavesSpaceship", dp, tp->get_gen_fn("LevelGen_IceCavesSpaceshipCnk"), 9, 12+1, true)));
	//saps.push_back(std::shared_ptr<StaticAreaPatch>(new StaticAreaPatch("IceCavesYeti", dp, tp->get_gen_fn("LevelGen_IceCavesYetiCnk"), 9, 12+1, true)));
	saps.push_back(std::shared_ptr<StaticAreaPatch>(new StaticAreaPatch("Worm", dp, tp->get_gen_fn("LevelGen_WormCnk"), 5, 12+1, true, 46)));
	saps.push_back(std::shared_ptr<StaticAreaPatch>(new StaticAreaPatch("Temple", dp, tp->get_gen_fn("LevelGen_TempleCnk"), 13, 15+1)));
	saps.push_back(std::shared_ptr<StaticAreaPatch>(new StaticAreaPatch("TempleOlmec", dp, tp->get_gen_fn("LevelGen_OlmecCnk"), 16, 16+1, true)));
	saps.push_back(std::shared_ptr<StaticAreaPatch>(new StaticAreaPatch("Hell", dp, tp->get_gen_fn("LevelGen_HellCnk"), 17, 20+1)));

	bool suc = false;
	for(auto&& sap : saps) {
		if(!sap->valid()) {
			DBG_EXPR(std::cout << "[StaticChunkPatch] Warning: SAP " << sap->get_name() << " failed." << std::endl);
			return;
		}
		else {
			suc = true;
		}
	}

	//if none of the SAPs succeeded, invalid.
	if(!suc) {
		DBG_EXPR(std::cout << "[StaticChunkPatch] All SAPs failed." << std::endl);
		is_valid = false;
	}
}

bool StaticChunkPatch::valid_tile(char tile) {
	return tp->valid_tile(tile);
}

bool StaticChunkPatch::valid() {
	return is_valid;
}

bool StaticChunkPatch::_perform() {
	yrp->perform();

	spel->write_mem(jngl_anticrash_addr, jngl_anticrash_mod, sizeof(jngl_anticrash_mod));

	for(auto&& sap : saps) {
		sap->perform();
		if(!sap->is_active()) {
			for(auto&& s : saps) {
				if(s.get() == sap.get())
					break;
				s->undo();
			}
			return false;
		}
	}
	return true;
}

bool StaticChunkPatch::_undo() {
	yrp->undo();

	spel->write_mem(jngl_anticrash_addr, jngl_anticrash_orig, sizeof(jngl_anticrash_mod));

	for(auto&& sap : saps) {
		sap->undo();
	}
	return true;
}

void StaticChunkPatch::apply_chunks() {
	for(auto&& sap : saps) {
		sap->apply_chunks();
	}
}

std::vector<Chunk*> StaticChunkPatch::get_chunks() {
	std::vector<Chunk*> out;
	
	for(auto&& sap : saps) {
		for(auto&& chunk : sap->get_chunks()) {
			out.push_back(chunk);
		}
	}

	return out;
}

std::vector<Chunk*> StaticChunkPatch::query_chunks(const std::string& start) {
	std::vector<Chunk*> out;

	for(auto&& sap : saps) {
		for(auto&& chunk : sap->query_chunks(start)) {
			out.push_back(chunk);
		}
	}

	return out;
}

std::vector<SingleChunk*> StaticChunkPatch::root_chunks() {
	std::vector<SingleChunk*> out;

	for(auto&& sap : saps) {
		for(auto&& chunk : sap->root_chunks()) {
			out.push_back(chunk);
		}
	}

	return out;
}

const std::set<char>& StaticChunkPatch::valid_tiles() {
	return tp->possible_tiles();
}

const std::shared_ptr<TilePatch> StaticChunkPatch::tile_patch() const {
	return tp;
}