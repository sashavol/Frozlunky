#include "append_ai.h"
#include <map>
#include <boost/assign.hpp>
#include <sstream>

#define EXEC_SPACE_ALLOC 512

BYTE ai_end_find[] = {0x83, 0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x7E, 0xFF, 0x81, 0x7F, 0xFF, 0xED, 0x03, 0x00, 0x00};
std::string ai_end_mask = "xx....xx.xx.xxxx";

BYTE ai_start_find[] = {0xD9, 0x47, 0xFF, 0xD9, 0x05, 0xFF, 0xFF, 0xFF, 0xFF, 0xDC, 0xC9, 0xD9, 0xC9, 0xE8, 0xFF, 0xFF, 0xFF, 0xFF, 0xD8, 0x4F, 0xFF};
std::string ai_start_mask = "xx.xx....xxxxx....xx.";

BYTE ai_entity_find[] = {0x3D, 0xAA, 0xAA, 0x00, 0x00, 0x75, 0xFF, 0x8B, 0xAA, 0xE8};
std::string ai_entity_mask = "x..xxx.x.x";

BYTE insert_jmp_find[] = {0x83,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x7E,0xCC,0x81,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x74,0xCC,0x80};
std::string insert_jmp_mask = "x......x.x......x.x";

//TODO
BYTE append_exec_code[]	= {0x50, 0x57, 0x56, 0x8B, 0xAA, 0xE8, 0xBB, 0xBB, 0xBB, 0xBB, 0x5E, 0x5F, 0x58};

//!TODO EAX is actually garbage, figure out the value which holds the entity id
//OPT This compares against EAX for entity ID, which is correct, but may become invalid in future spelunky versions
//param A (+4) : entity ID
//param B (+10) : je diff to target
BYTE check_entity_code[] = {0x8B, 0x47, 0x0C, 0x3D, 0xAA, 0xAA, 0xAA, 0xAA, 0x0F, 0x84, 0xBB, 0xBB, 0xBB, 0xBB}; 

#define INSERT_JMP_SIZE 7

AppendAIPatch::~AppendAIPatch() {
	if(exec_space != NULL) {
		this->_undo();
		spel->release(exec_space);
		exec_space = NULL;
	}

	if(original_insert_mem != nullptr) {
		delete[] original_insert_mem;
	}
}

AppendAIPatch::AppendAIPatch(std::shared_ptr<Spelunky> spel) : Patch(spel), 
		spel(spel), 
		is_valid(true),
		exec_space(NULL),
		original_insert_mem(nullptr)
{
	insert_addr = spel->find_mem(insert_jmp_find, insert_jmp_mask);
	if(insert_addr == 0x0) {
		is_valid = false;
		return;
	}

	original_insert_mem = new BYTE[INSERT_JMP_SIZE];
	spel->read_mem(insert_addr, original_insert_mem, INSERT_JMP_SIZE);

	Address start = spel->find_mem(ai_start_find, ai_start_mask);
	Address end = spel->find_mem(ai_end_find, ai_end_mask, start);
	exec_space = spel->allocate(EXEC_SPACE_ALLOC, true);

#ifdef DEBUG_MODE
	std::cout << "[AppendAI] Inserting jmp @ " << std::setbase(16) << insert_addr << std::endl;
	std::cout << "[AppendAI] Allocated executable space @ " << std::setbase(16) << exec_space << std::endl;
	std::cout << "[AppendAI] Found entity ai patch start @ " << std::setbase(16) << start << std::endl;
	std::cout << "[AppendAI] Found entity ai patch end @ " << std::setbase(16) << end << std::endl;
#endif

	Address current = start+1;

	while((current = spel->find_mem(ai_entity_find, ai_entity_mask, current)) != 0x0) {
		entity_id entity_id;
		spel->read_mem(current+0x1, &entity_id, sizeof(entity_id));
		
		//if(entity_id >= 0x3E9) {
			BYTE movtype;
			spel->read_mem(current+0x8, &movtype, sizeof(BYTE));

			Address behave_addr;
			spel->read_mem(current+0xA, &behave_addr, sizeof(Address));
			behave_addr += (current+0x9+0x5);

			entity_data[entity_id] = EntityType(entity_id, movtype, behave_addr);

	#ifdef DEBUG_MODE
			std::cout << "Found (possible) entity id " << std::setbase(16) << entity_id << ", movtype = " << (int)movtype << ", behave_addr @ " 
				<< std::setbase(16) << behave_addr << std::endl;
	#endif
			available_ids.push_back(entity_id);
		//}

		current++;
	}	
}


bool AppendAIPatch::_perform() {
	Address end = exec_space+sizeof(check_entity_code)*ignored.size()+sizeof(append_exec_code)*current_appended.size();

	int offset = 0;
	for(unsigned entity : ignored) {
		unsigned diff = je_diff(exec_space+offset+8, end);

		spel->write_mem(exec_space+offset, check_entity_code, sizeof(check_entity_code));
		spel->write_mem(exec_space+offset+4, &entity, sizeof(unsigned));
		spel->write_mem(exec_space+offset+10, &diff, sizeof(unsigned));

		offset += sizeof(check_entity_code);
	}

	for(unsigned entity_id : current_appended) {
		EntityType entity = entity_data[entity_id];
		spel->write_mem(exec_space+offset, append_exec_code, sizeof(append_exec_code));
		spel->write_mem(exec_space+offset+4, &entity.movtype, sizeof(BYTE));

		unsigned diff = call_diff(exec_space+offset+5, entity.behave_addr);
		spel->write_mem(exec_space+offset+6, &diff, sizeof(unsigned));
	
		offset += sizeof(append_exec_code);
	}

	this->_undo();
	spel->jmp_build(insert_addr, INSERT_JMP_SIZE, exec_space, offset);
	return true;
}


bool AppendAIPatch::_undo() {
	spel->write_mem(insert_addr, original_insert_mem, INSERT_JMP_SIZE);
	return true;
}



bool AppendAIPatch::set_appended_ai(const entity_storage& entity) {
	current_appended = entity;

	if(this->is_active()) {
		this->_perform();
	}

	return true;
}

AppendAIPatch::entity_storage AppendAIPatch::appended_ai() {
	return current_appended;
}



std::map<unsigned, std::string> friendly_names = boost::assign::map_list_of
	(ENTITY_FROG, "Blue Frog")
	(ENTITY_ORANGEFROG, "Orange Frog")
	(ENTITY_BIG_FROG, "Big Frog")
	(ENTITY_COBRA, "Cobra")
	(ENTITY_SNAKE, "Snake")
	(ENTITY_ANUBIS22, "Anubis 2")
	(ENTITY_ANUBIS2, "Anubis 2")
	(ENTITY_ANUBIS, "Anubis")
	(ENTITY_SPIDER3, "Spider 1")
	(ENTITY_SPIDER2, "Spider 2")
	(ENTITY_SPIDER, "Spider 3")
	(ENTITY_BAT, "Bat")
	(ENTITY_DAMSEL, "Damsel")
	(ENTITY_UFO, "UFO")
	(ENTITY_PIRANHA, "Piranha")
	(ENTITY_VOMITFLIES, "Vomit Flies")
	(ENTITY_YETI, "Yeti")
	(ENTITY_CHIMP, "Chimp")
	(ENTITY_GHOST, "Ghost")
	(ENTITY_ALIEN, "Little Alien")
	(ENTITY_BLUEDEVIL, "Blue Devil")
	(ENTITY_CAVEMAN, "Caveman")
	(ENTITY_CAVEMAN2, "Caveman 2")
	(ENTITY_IMP, "Imp")
	(ENTITY_SCORPION, "Scorpion")
	(ENTITY_BEE, "Bee")
	(ENTITY_BEE2, "Bee 2")
	(ENTITY_MAMMOTH, "Mammoth")
	(ENTITY_TANK, "Tank")
	(ENTITY_SNAIL, "Snail")
	(ENTITY_ALIEN_QUEEN, "Alien Queen")
	(ENTITY_TURRET, "Turret")
	(ENTITY_YAMA, "King Yama")
	(ENTITY_PENGUIN, "Penguin Critter")
	(ENTITY_FROG_CRITTER, "Frog Critter")
	(ENTITY_SHOPKEEPER, "Shopkeeper")
	(ENTITY_BACTERIA, "Bacteria")
	(ENTITY_SCORPION_FLY, "Scorpion Fly")
	(ENTITY_HAWK_MAN, "Hawk Man")
	(ENTITY_WORM_BABY, "Worm Baby")
	(ENTITY_BLACK_KNIGHT, "Black Knight")
	(ENTITY_FISH_CRITTER, "Fish Critter")
	(ENTITY_VAMPIRE, "Vampire")
	(ENTITY_SUCCUBUS, "Succubus")
	(ENTITY_HORSEHEAD, "Horse Head")
	(ENTITY_OXFACE, "Ox Face");

std::string AppendAIPatch::FriendlyName(unsigned entity) {
	auto iter = friendly_names.find(entity);
	if(iter != friendly_names.end()) {
		return iter->second;
	}
	else {
		std::stringstream ss;
		ss << std::setbase(16) << entity;
		return std::string("[")+ss.str()+"] (Unknown)";
	}
}