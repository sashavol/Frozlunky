#pragma once

#include <map>
#include <vector>
#include <string>
#include <boost/assign.hpp>

namespace KnownEntities {
	typedef std::vector<std::pair<int, std::string> > map_type;

	const map_type& All();
	std::string GetName(int entity);
	int GetID(const std::string& name);

	std::vector<std::string> Search(const std::string& query);
}