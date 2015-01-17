#pragma once

#include <map>
#include <vector>
#include <string>
#include <boost/assign.hpp>

#define ARROW_TRAP_LEFT_FACING (1 << 15)

#define W_TILE_BG_FLAG (1 << 16)
#define ENTITY_FLAGS (W_TILE_BG_FLAG)

int raw_entity(int entity);

namespace KnownEntities {
	typedef std::vector<std::pair<int, std::string> > map_type;

	const map_type& All();
	std::string GetName(int entity);
	int GetID(const std::string& name);
	unsigned GetColor(int entity);

	std::vector<std::string> Search(const std::string& query);
}