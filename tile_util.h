#pragma once

#include <string>
#include <vector>

namespace TileUtil {
	std::string QueryTileFile(bool save);
	std::string GetBaseFilename(const std::string& addr);

	std::vector<std::string> Search(const std::vector<std::string>& list, const std::string& query);
}