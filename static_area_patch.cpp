#include "static_area_patch.h"
#include <algorithm>

#define SUBROUTINE_ALLOC 0x4000

/*
	AA (+2)  -> Register type
	BB (+5)  -> Current game ptr
	CC (+11) -> Level offset
*/
static BYTE base_opcode[] = {0x60,0x8B,0xAA,0x8B,0x35,0xBB,0xBB,0xBB,0xBB,0x8B,0x96,0xCC,0xCC,0xCC,0xCC};

/*
	AA (+2)  -> Level index
	BB (+5)  -> Diff to next lvlseg
*/
static BYTE lvlseg_opcode[] = {0x83,0xFA,0xAA,0x0F,0x85,0xBB,0xBB,0xBB,0xBB};

/*
	AA (+2)  -> Chunk index 
	BB (+7)  -> Chunk string pointer
	CC (+4)  -> Diff to next cnkseg
*/
static BYTE cnkseg_opcode[] = {0x83,0xF8,0xAA,0x75,0xCC,0x61,0x68,0xAA,0xAA,0xAA,0xAA};

/*
	AA (+1)  -> Jmpout diff
*/
static BYTE cnkseg_jmpout[] = {0xE9,0xAA,0xAA,0xAA,0xAA};

/*
	AA (+1) -> Builder call diff
*/
static BYTE builder_call[] = {0xE8,0xAA,0xAA,0xAA,0xAA};

/*
	AA (+3) -> Current game ptr
	BB (+9) -> Dark level offset
	CC (+13) -> Dark value bool
*/
static BYTE dark_set[] = {0x60, 0x8B,0x35,0xAA,0xAA,0xAA,0xAA, 0xC6, 0x86, 0xBB,0xBB,0xBB,0xBB, 0xCC, 0x61};

static Address find_end(std::shared_ptr<Spelunky> spel, Address start_addr) {
	static BYTE end_find[] = {0x83, 0xC4, 0xCC, 0xC3};
	static BYTE end2_find[] = {0x83, 0xC4, 0xCC, 0xC2};
	static std::string end_mask = "xx.x";

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
	}

	return end;
}


struct jumpfind {
	Address jumpto;
	Address pushref;
	Address jmpref;

	jumpfind(Address jumpto, Address pushref, Address jmpref) : 
		jumpto(jumpto), pushref(pushref), jmpref(jmpref) 
	{}
};

//converts all jumps and calls to their static address, 
//use extract_pre to compute diff for the modified addresses
static void process_pre(Address pre_base, BYTE* pre, size_t presize) {
	for(size_t p = 0; p < presize;) {
		if(pre[p] == 0xE9 || pre[p] == 0xE8) {
			*(signed int*)(pre + p + 1) += (pre_base + p + 5);
			p += 5;
		}
		else
			p++;
	}
}

static void extract_pre(Address ex, BYTE* pre, size_t presize) {
	for(size_t p = 0; p < presize;) {
		if(pre[p] == 0xE9 || pre[p] == 0xE8) {
			*(signed int*)(pre + p + 1) -= (ex + p + 5);
			p += 5;
		}
		else
			p++;
	}
}

bool StaticAreaPatch::find_jmp() {
	//OPT issue: unsafe assumption about instruction base

	static BYTE push_find[] = {0x68, 0xCC,0xCC,0xCC,0xCC};
	static std::string push_mask = "x....";

	static BYTE shortjmp_find[] = {0xEB, 0xCC};
	static std::string shortjmp_mask = "x.";

	static BYTE longjmp_find[] = {0xE9, 0xCC, 0xCC, 0xCC, 0xCC};
	static std::string longjmp_mask = "x....";

	std::vector<jumpfind> jfs;

	for(Address psh = spel->find_mem(push_find, push_mask, gen_fn); psh != 0 && psh < gen_fn_end; psh = spel->find_mem(push_find, push_mask, psh+1)) {
		Address jms = spel->find_mem(shortjmp_find, shortjmp_mask, psh);
		Address jml = spel->find_mem(longjmp_find, longjmp_mask, psh);
		if(jms < jml) {
			signed char diff = 0;
			spel->read_mem(jms, &diff, sizeof(signed char));
			Address targ = (jms + 2) + diff;
			jfs.push_back(jumpfind(targ, psh, jms));
		}
		else {
			signed int diff = 0;
			spel->read_mem(jml + 1, &diff, sizeof(signed int));
			Address targ = (jml + 5) + diff;
			jfs.push_back(jumpfind(targ, psh, jml));
		}
	}

	char cnkcheck[CHUNK_LEN];
	std::map<Address, size_t> jmpcounts;
	std::map<Address, std::map<size_t, size_t>> diff_counts;
	for(auto&& jf : jfs) {
		Address cnk;
		spel->read_mem(jf.pushref + 1, &cnk, sizeof(Address));
		spel->read_mem(cnk, cnkcheck, sizeof(cnkcheck));

		if(strlen(cnkcheck) == CHUNK_LEN-1 && jf.jumpto >= jf.pushref && jf.jumpto < gen_fn_end) {
			diff_counts[jf.jumpto][jf.jmpref - (jf.pushref + 5)]++;
			jmpcounts[jf.jumpto]++;
		}
	}

	Address jmpout = std::max_element(jmpcounts.begin(), jmpcounts.end(), 
		[=](const std::pair<Address, size_t>& a, const std::pair<Address, size_t>& b) {
			if(a.second < 5 && b.second >= 5)
				return true;
			else
				return a.first > b.first;
		}
	)->first;
	
	
	auto& diffc = diff_counts[jmpout];
	auto maxdiff = std::max_element(diffc.begin(), diffc.end(), 
		[=](const std::pair<size_t, size_t>& a, const std::pair<size_t, size_t>& b) {
			return a.second < b.second;
		}
	);

	
	//OPT issue: unsafe
	//mov esi, [esp+3C (+4)], add 4 to offset string push
	static BYTE jungle_esp_fix[] = {0x8B, 0x74, 0x24, 0x3C + 4};

	for(auto&& jf : jfs) {
		if(jf.jumpto == jmpout && (jf.jmpref - (jf.pushref + 5)) == maxdiff->first) {
			jmpout_addr = jf.jumpto;
			
			BYTE* jpre;
			if(name == "Jungle") {
				jmpout_pre_size = sizeof(jungle_esp_fix) + jf.jmpref - (jf.pushref + 5);
				jmpout_pre = new BYTE[jmpout_pre_size];
				std::memcpy(jmpout_pre, jungle_esp_fix, sizeof(jungle_esp_fix));
				jpre = jmpout_pre + sizeof(jungle_esp_fix);
			}
			else {
				jmpout_pre_size = jf.jmpref - (jf.pushref + 5);
				jmpout_pre = new BYTE[jmpout_pre_size];
				jpre = jmpout_pre;
			}
			
			spel->read_mem(jf.pushref + 5, jpre, jmpout_pre_size);
			process_pre(jf.pushref + 5, jpre, jmpout_pre_size);
			return true;
		}
	}

	return false;
}

//finds where to insert, and how many bytes the overrided instruction is
std::pair<Address, size_t> StaticAreaPatch::find_insert() {
	//or ..., 0xFFFFFFFF
	//cmp ..., 4
	//+0 (o 6)
	static BYTE generic_find[] = {0x83,0xCC,0xFF,0x83,0xCC,0x04};
	static std::string generic_mask = "x.xx.x";
	
	//WARN This insertion hook will cause a crash if the subroutine hits the end due to short jump
	//+0 (o 8)
	static BYTE olmec_find[] = {0x75,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0x38};
	static std::string olmec_mask = "x.x.....x";

	//+0 (o 6)
	static BYTE tutorial_find[] = {0x81,0xCC,0xE8,0x03,0x00,0x00,0x75};
	static std::string tutorial_mask = "x.xxxxx";


	{
		Address gen = spel->find_mem(generic_find, generic_mask, gen_fn);
		if(gen && gen < gen_fn_end)
			return std::pair<Address, size_t>(gen, 6);
	}
	{
		Address olm = spel->find_mem(olmec_find, olmec_mask, gen_fn);
		if(olm && olm < gen_fn_end)
			return std::pair<Address, size_t>(olm, 8);
	}
	{
		Address tut = spel->find_mem(tutorial_find, tutorial_mask, gen_fn);
		if(tut && tut < gen_fn_end)
			return std::pair<Address, size_t>(tut, 6);
	}
	
	return std::pair<Address, size_t>(0, 0);
}

#define CMP2MOV_OFFSET -0x38
BYTE StaticAreaPatch::find_cidx_register() {
	//or ..., 0xFFFFFFFF
	//cmp ..., 4
	//+0 (o 6)
	static BYTE generic_find[] = {0x83,0xCC,0xFF,0x83,0xCC,0x04};
	static std::string generic_mask = "x.xx.x";

	{
		Address gen = spel->find_mem(generic_find, generic_mask, gen_fn);
		if(gen && gen < gen_fn_end) {
			BYTE cmpb = 0;
			spel->read_mem(gen + 4, &cmpb, sizeof(BYTE));
			if((int)cmpb > -CMP2MOV_OFFSET) //TODO this is messing up ?
				return cmpb + CMP2MOV_OFFSET;
			else
				return 0;
		}
	}

	//OPT these are assumptions based on the generator type, may be unsafe for future versions.
	if(name.find("Tutorial") == 0) {
		return 0xC3; //mov eax, ebx
	}
	else if(name.find("TempleOlmec") == 0) {
		return 0xC1; //mov eax, ecx
	}

	return 0;
}

StaticAreaPatch::~StaticAreaPatch() {
	if(chunk_alloc) {
		spel->free(chunk_alloc);
		chunk_alloc = 0;
	}

	if(subroutine_alloc) {
		spel->free(subroutine_alloc);
		subroutine_alloc = 0;
	}

	if(insert_orig) {
		delete[] insert_orig;
		insert_orig = nullptr;
	}

	for(SingleChunk* sc : *chunks) {
		delete sc;
	}

	delete chunks;
}

StaticAreaPatch::StaticAreaPatch(const std::string& name, std::shared_ptr<GameHooks> gh, Address gen_fn, int lvl_start, int lvl_end, bool single_level, int lvl_chunks) : 
	Patch(gh->spel),
	dp(gh->dp),
	gh(gh),
	gen_fn(gen_fn),
	lvl_start(lvl_start),
	lvl_end(lvl_end),
	chunk_alloc(0),
	subroutine_alloc(0),
	insert_addr(0),
	gen_fn_end(0),
	insert_orig(nullptr),
	name(name),
	lvl_chunks(lvl_chunks),
	jmpout_pre(nullptr),
	jmpout_pre_size(0),
	jmpout_addr(0),
	is_valid(true),
	chunks(new std::vector<SingleChunk*>())
{
	if(!gen_fn) {
		DBG_EXPR(std::cout << "[StaticAreaPatch] Passed null gen_fn." << std::endl);
		is_valid = false;
		return;
	}

	gen_fn_end = find_end(spel, gen_fn);
	if(!gen_fn_end) {
		DBG_EXPR(std::cout << "[StaticAreaPatch] Failed to find gen function end." << std::endl);
		is_valid = false;
		return;
	}

	subroutine_alloc = spel->allocate(SUBROUTINE_ALLOC, true);
	if(!subroutine_alloc) {
		DBG_EXPR(std::cout << "[StaticAreaPatch] Failed to allocate subroutine memory." << std::endl);
		is_valid = false;
		return;
	}
	DBG_EXPR(std::cout << "[StaticAreaPatch] (Levels " << lvl_start << " -> " << lvl_end << ")  Allocated subroutine at " << subroutine_alloc << std::endl);

	if(!single_level) {
		chunk_alloc = spel->allocate(CHUNK_LEN * (lvl_end - lvl_start) * lvl_chunks + 1);	
	}
	else {
		chunk_alloc = spel->allocate(CHUNK_LEN * lvl_chunks + 1);
	}

	if(!chunk_alloc) {
		DBG_EXPR(std::cout << "[StaticAreaPatch] Failed to allocate chunk memory." << std::endl);
		is_valid = false;
		return;
	}

	//construct entity spawn builders / dark status
	if(!single_level) {
		for(int lvl = lvl_start; lvl < lvl_end; ++lvl) {
			std::shared_ptr<EntitySpawnBuilder> builder = std::make_shared<EntitySpawnBuilder>(dp);
			if(!builder->valid()) {
				DBG_EXPR(std::cout << "[StaticAreaPatch] Failed to construct valid entity builder." << std::endl);
				is_valid = false;
				return;
			}
			builders[lvl] = builder;
			dark_levels[lvl] = std::make_shared<bool>(false);
		}
	}
	else {
		std::shared_ptr<EntitySpawnBuilder> builder = std::make_shared<EntitySpawnBuilder>(dp);
		if(!builder->valid()) {
			DBG_EXPR(std::cout << "[StaticAreaPatch] Failed to construct valid entity builder." << std::endl);
			is_valid = false;
			return;
		}
		
		std::shared_ptr<bool> dark = std::make_shared<bool>(false);
		for(int lvl = lvl_start; lvl < lvl_end; ++lvl) {
			builders[lvl] = builder;
			dark_levels[lvl] = dark;
		}
	}

	//TODO construct Chunk* and store into internal chunk vector
	{
		BYTE empty_chunk[CHUNK_LEN];
		std::fill(empty_chunk, empty_chunk+CHUNK_LEN, '0');
		empty_chunk[CHUNK_LEN-1] = 0;

		if(!single_level) {
			for(int lvl = lvl_start; lvl < lvl_end; lvl++) {
				Address lvl_alloc = chunk_alloc + (lvl-lvl_start)*CHUNK_LEN*lvl_chunks;
				
				for(int c = 0; c < lvl_chunks; c++) {
					std::string cnk_name = name + "-" + std::to_string(lvl) + "-" + std::to_string(c);
					spel->write_mem(lvl_alloc + CHUNK_LEN*c, empty_chunk, sizeof(empty_chunk));
					named_allocs[cnk_name] = lvl_alloc + CHUNK_LEN*c;

					SingleChunk* cnk = new SingleChunk(cnk_name, std::string((char*)empty_chunk), CHUNK_WIDTH, CHUNK_HEIGHT);
					chunks->push_back(cnk);

					level_parents[static_cast<Chunk*>(cnk)] = lvl;
				}
				
				allocs[lvl] = lvl_alloc;
			}
		}
		else {
			for(int c = 0; c < lvl_chunks; c++) {
				spel->write_mem(chunk_alloc + CHUNK_LEN*c, empty_chunk, sizeof(empty_chunk));
				std::string cnk_name = name + "-" + std::to_string(c);
				named_allocs[cnk_name] = chunk_alloc + CHUNK_LEN*c;

				SingleChunk* cnk = new SingleChunk(cnk_name, std::string((char*)empty_chunk), CHUNK_WIDTH, CHUNK_HEIGHT);
				chunks->push_back(cnk);

				level_parents[static_cast<Chunk*>(cnk)] = lvl_start;
			}

			for(int lvl = lvl_start; lvl < lvl_end; lvl++) {
				allocs[lvl] = chunk_alloc;
			}
		}
	}
	
	if(!find_jmp()) {
		DBG_EXPR(std::cout << "[StaticAreaPatch] Discovered chunk generation jumpout is invalid." << std::endl);
		is_valid = false;
		return;
	}

	std::pair<Address, size_t> insert = find_insert();
	if(!insert.first) {
		DBG_EXPR(std::cout << "[StaticAreaPatch] Failed to gen insert pos." << std::endl);
		is_valid = false;
		return;
	}

	insert_addr = insert.first;
	insert_size = insert.second;

	insert_orig = new BYTE[insert_size];
	spel->read_mem(insert_addr, insert_orig, insert_size);

	eaxmov_opcode = find_cidx_register();
	if(!eaxmov_opcode) {
		DBG_EXPR(std::cout << "[StaticAreaPatch] Failed to identify chunk index register." << std::endl);
		is_valid = false;
		return;
	}

	DBG_EXPR(std::cout << "[StaticAreaPatch] Inserting at " << insert.first << " size " << insert.second << std::endl);
}

const std::string& StaticAreaPatch::get_name() {
	return name;
}

bool StaticAreaPatch::valid() {
	return is_valid;
}

bool StaticAreaPatch::_perform() {
	Address sr = subroutine_alloc;
	
	//base
	spel->write_mem(sr, base_opcode, sizeof(base_opcode));
	spel->write_mem(sr+2, &eaxmov_opcode, sizeof(BYTE));
	
	const Address game_ptr = dp->game_ptr();
	const Address level_offs = dp->current_level_offset();
	spel->write_mem(sr+5, &game_ptr, sizeof(Address));
	spel->write_mem(sr+11, &level_offs, sizeof(Address));

	sr += sizeof(base_opcode);
	
	BYTE* prej = new BYTE[jmpout_pre_size];
	//lvlseg->cnkseg[]->lvlseg->...
	for(int lvl = lvl_start; lvl < lvl_end; lvl++) {
		BYTE blvl = (BYTE)lvl;
		spel->write_mem(sr, lvlseg_opcode, sizeof(lvlseg_opcode));
		spel->write_mem(sr+ 2, &blvl, sizeof(BYTE));

		signed int cnkseg_size = sizeof(cnkseg_opcode)+sizeof(cnkseg_jmpout)+jmpout_pre_size;
		signed int diff = (sr + sizeof(lvlseg_opcode) + sizeof(builder_call) + sizeof(dark_set) + lvl_chunks*cnkseg_size) - (sr+3+6);
		spel->write_mem(sr + 5, &diff, sizeof(signed int));

		sr += sizeof(lvlseg_opcode);

		Address lvl_alloc = allocs[lvl];
		for(int c = 0; c < lvl_chunks; c++) {
			BYTE bc = (BYTE)c;
			Address cnk_alloc = lvl_alloc+CHUNK_LEN*c;
			
			spel->write_mem(sr, cnkseg_opcode, sizeof(cnkseg_opcode));
			spel->write_mem(sr+2, &bc, sizeof(BYTE));
			spel->write_mem(sr+7, &cnk_alloc, sizeof(Address));

			signed char cnkdiff = (signed char)cnkseg_size - 5;
			if(c == 0) {
				cnkdiff += sizeof(builder_call);
				cnkdiff += sizeof(dark_set);
			}
			spel->write_mem(sr+4, &cnkdiff, sizeof(signed char));

			sr += sizeof(cnkseg_opcode);
			
			//entity spawning + set dark level
			if(c == 0) {
				//call entity builder
				signed int calldiff = builders[lvl]->subroutine_addr() - (sr + 5);
				spel->write_mem(sr, builder_call, sizeof(builder_call));
				spel->write_mem(sr+1, &calldiff, sizeof(Address));
				sr += sizeof(builder_call);

				//set dark level flag
				signed int dark_offs = gh->dark_level_offset();
				char val = (*(dark_levels[lvl])) ? 1 : 0;
				spel->write_mem(sr, dark_set, sizeof(dark_set));
				spel->write_mem(sr+3, &game_ptr, sizeof(Address));
				spel->write_mem(sr+9, &dark_offs, sizeof(signed int));
				spel->write_mem(sr+13, &val, sizeof(char));
				dark_level_valaddrs[lvl] = sr+13;

				sr += sizeof(dark_set);
			}
			
			std::memcpy(prej, jmpout_pre, jmpout_pre_size);
			extract_pre(sr, prej, jmpout_pre_size);
			spel->write_mem(sr, prej, jmpout_pre_size);

			sr += jmpout_pre_size;

			signed int jout_diff = jmpout_addr - (sr+5);
			spel->write_mem(sr, cnkseg_jmpout, sizeof(cnkseg_jmpout));
			spel->write_mem(sr+1, &jout_diff, sizeof(signed int));

			sr += sizeof(cnkseg_jmpout);
		}
	}
	delete[] prej;

	spel->jmp_build(insert_addr, insert_size, subroutine_alloc, sr - subroutine_alloc);
	return true;
}

bool StaticAreaPatch::_undo() {
	spel->write_mem(insert_addr, insert_orig, insert_size);
	return true;
}

int StaticAreaPatch::level_start() {
	return lvl_start;
}

int StaticAreaPatch::level_end() {
	return lvl_end;
}

std::vector<Chunk*> StaticAreaPatch::query_chunks(const std::string& start) {
	std::vector<Chunk*> out;
	for(auto&& sc : *chunks) {
		if(sc->get_name().find(start) == 0)
			out.push_back(sc);
	}
	return out;

}

std::vector<Chunk*> StaticAreaPatch::get_chunks() {
	std::vector<Chunk*> out;
	for(auto&& sc : *chunks)
		out.push_back(sc);
	return out;
}

std::vector<SingleChunk*> StaticAreaPatch::root_chunks() {
	return *chunks;
}

void StaticAreaPatch::apply_chunks() {
	for(auto&& sc : *chunks) {
		Address addr = named_allocs[sc->get_name()];
		if(!addr) {
			DBG_EXPR(std::cout << "[StaticAreaPatch] Warning: Encountered non-native chunk during apply: " << sc->get_name() << std::endl);
		}
		else {
			spel->write_mem(addr, sc->get_data().c_str(), CHUNK_LEN);
		}
	}

	std::set<EntitySpawnBuilder*> updated;
	for(auto&& builder : builders) {
		if(updated.find(builder.second.get()) == updated.end()) {
			builder.second->update_memory();
			updated.insert(builder.second.get());
		}
	}

	for(auto&& lvl : dark_levels) {
		char val = *lvl.second ? 1 : 0;
		spel->write_mem(dark_level_valaddrs[lvl.first], &val, sizeof(char));
	}
}

std::shared_ptr<bool> StaticAreaPatch::dark_status(int lvl) {
	return dark_levels[lvl];
}

std::shared_ptr<EntitySpawnBuilder> StaticAreaPatch::entity_builder(int lvl) {
	return builders[lvl];
}

int StaticAreaPatch::identify_chunk(Chunk* cnk) {
	auto p = level_parents.find(cnk);
	if(p != level_parents.end()) {
		return p->second;
	}
	else
		return -1;
}