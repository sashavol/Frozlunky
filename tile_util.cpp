#include <stdexcept>
#include <Windows.h>
#include "tile_util.h"

namespace TileUtil {
	std::string QueryTileFile(bool save) {
		char szFile[MAX_PATH];
		szFile[0] = '\0';

		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
	
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "XML Chunk Format (.xml)\0*.xml\0";
		
		ofn.Flags  = OFN_PATHMUSTEXIST;
		if(!save) {
			ofn.Flags |= OFN_FILEMUSTEXIST;
		}

		BOOL res;
		if(save)
			res = GetSaveFileName(&ofn);
		else
			res = GetOpenFileName(&ofn);

		if(!res) {
			throw std::runtime_error("File open failed.");
		}

		std::string ret(szFile);
		if(ret.find(".xml") != ret.size() - 4) {
			ret += ".xml";
		}

		return ret;
	}

	std::string GetBaseFilename(const std::string& addr) {
		return addr.substr(addr.find_last_of("/")+1, addr.size());
	}
}