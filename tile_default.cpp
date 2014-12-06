#include "tile_default.h"
#include "tile_editing.h"
#include <map>
#include <boost/assign.hpp>

namespace TileDefault {
	static std::map<std::string, std::string> defaults; //TODO

	static void ToDefault_(Chunk* cnk)  {
		if(cnk->type() == ChunkType::Group) {
			for(Chunk* c : static_cast<GroupChunk*>(cnk)->get_chunks()) {
				ToDefault_(c);
			}
		}
		else {
			SingleChunk* sc = static_cast<SingleChunk*>(cnk);
			auto d = defaults.find(sc->get_name());
			
			//try to find the base chunk in defaults, otherwise set the chunk to blank
			if(d != defaults.end()) {
				sc->set_data(d->second);
			}
			else {
				for(int y = 0, ym = sc->get_height(); y < ym; y++) {
					for(int x = 0, xm = sc->get_width(); x < xm; x++) {
						sc->tile(x, y, '0');
					}
				}
			}
		}
	}

	void SetToDefault(const std::vector<Chunk*>& chunks) {
		for(Chunk* c : chunks) {
			ToDefault_(c);
		}
	}
}