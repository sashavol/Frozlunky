#include "tile_draw.h"
#include "tile_chunk.h"

namespace Description {
	std::string TileDescription(char tile, AreaRenderMode mode);
	std::string TileDescription(char tile, Chunk* parent);
	std::string ChunkDescription(Chunk* cnk);
}