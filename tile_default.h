#pragma once

#include <vector>
#include "tile_chunk.h"
#include "entity_spawn_builder.h"

namespace TileDefault {
	void SetToDefault(const std::vector<Chunk*>& chunks);
	void SetEntitiesToDefault(const std::string& area, std::shared_ptr<EntitySpawnBuilder>& esb);
};