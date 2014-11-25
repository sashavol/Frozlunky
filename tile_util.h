#pragma once

#include <string>

namespace TileUtil {
	std::string QueryTileFile(bool save);
	std::string GetBaseFilename(const std::string& addr);
}