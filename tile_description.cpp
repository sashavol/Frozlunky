#include "tile_description.h"
#include <map>
#include <boost/assign.hpp>

using namespace boost::assign;

//defaults to previous area if not defined
static std::map<char, std::map<AreaRenderMode, std::string>> descriptions = map_list_of
	('!', map_list_of
		(TUTORIAL, "Tutorial background billboard")
		(MINES, "Unused"))
	('#', map_list_of
		(TUTORIAL, "TNT")
		(JUNGLE, "Hell bible"))
	('$', map_list_of
		(TUTORIAL, "Roulette item"))
	('%', map_list_of
		(TUTORIAL, "Roulette item barricade"))
	('&', map_list_of
		(TUTORIAL, "Waterfall")
		(TEMPLE, "Lavafall"))
	('*', map_list_of
		(TUTORIAL, "Spinning spike ball"))
	('+', map_list_of
		(TUTORIAL, "Wooden background")
		(JUNGLE, "Inactive, causes crash")
		(WORM, "Wooden background")
		(ICE_CAVES, "Alien spaceship tile")
		(TEMPLE, "Wooden background"))
	(',', map_list_of
		(TUTORIAL, "Probabilistic terrain/wood tile"))
	('-', map_list_of
		(TUTORIAL, "Cracking ice platform"))
	('.', map_list_of
		(TUTORIAL, "Unmodified terrain tile"))
	('0', map_list_of
		(TUTORIAL, "Empty tile"))
	('1', map_list_of
		(TUTORIAL, "Regular terrain tile"))
	('2', map_list_of
		(TUTORIAL, "Probabilistic terrain/empty tile"))
	('3', map_list_of
		(TUTORIAL, "Probabilistic terrain/water tile")
		(TEMPLE, "Probabilistic terrain/lava tile"))
	('4', map_list_of
		(TUTORIAL, "Push block with physics")
		(ICE_CAVES, "Crashes game")
		(TEMPLE, "Push block with physics"))
	('5', map_list_of
		(TUTORIAL, "Obstacle marker tile 1"))
	('6', map_list_of
		(TUTORIAL, "Obstacle marker tile 2"))
	('7', map_list_of
		(TUTORIAL, "Probabilistic spike/empty tile")
		(JUNGLE, "Causes crash")
		(WORM, "Probabilistic spike/empty tile"))
	('8', map_list_of
		(TUTORIAL, "Terrain-generating entrance/exit with platform"))
	('9', map_list_of
		(TUTORIAL, "Static platformless entrance/exit"))
	(':', map_list_of
		(TUTORIAL, "Scorpion spawn")
		(JUNGLE, "Tikiman/caveman spawn")
		(WORM, "Area enemy spawn")
		(ICE_CAVES, "Yeti spawn"))
	(';', map_list_of
		(TUTORIAL, "Idol + Damsel side-by-side spawn")
		(ICE_CAVES, "'Psychic presence' triple force field")
		(TEMPLE, "Idol + Damsel side-by-side spawn"))
	('=', map_list_of
		(TUTORIAL, "Wooden platform"))
	('?', map_list_of
		(TUTORIAL, "Inactive"))
	('A', map_list_of
		(TUTORIAL, "Idol altar tile"))
	('B', map_list_of
		(TUTORIAL, "Idol trap stone platform tile"))
	('C', map_list_of
		(TUTORIAL, "Non-movable push block")
		(HELL, "Crate"))
	('D', map_list_of
		(TUTORIAL, "Damsel")
		(MINES, "Inactive")
		(HAUNTED_CASTLE, "Closing castle door tile")
		(WORM, "Inactive"))
	('E', map_list_of
		(TUTORIAL, "Gold bar spawn")
		(MINES, "Probabilistic chest/crate/terrain/empty tile"))
	('F', map_list_of
		(TUTORIAL, "Inactive")
		(ICE_CAVES, "Probabilistic self+neighbor ice/platform/empty generator"))
	('G', map_list_of
		(TUTORIAL, "Ladder tile"))
	('H', map_list_of
		(TUTORIAL, "Ladder platform junction"))
	('I', map_list_of
		(TUTORIAL, "Idol spawn tile")
		(HELL, "Tiki spawn"))
	('J', map_list_of
		(TUTORIAL, "Ol' Bitey spawn"))
	('K', map_list_of
		(TUTORIAL, "Shopkeeper spawn"))
	('L', map_list_of
		(TUTORIAL, "Ladder tile")
		(JUNGLE, "Vine tile")
		(WORM, "Climbable uvula tile")
		(ICE_CAVES, "Upward laser tile")
		(TEMPLE, "Ladder tile")
		(HELL, "Chain tile"))
	('M', map_list_of
		(TUTORIAL, "Terrain block containing mattock")
		(ICE_CAVES, "Terrain block containing jetpack")
		(TEMPLE, "Terrain block containing mattock"))
	('N', map_list_of
		(TUTORIAL, "Snake spawn"))
	('O', map_list_of
		(TUTORIAL, "Moai head spawn"))
	('P', map_list_of
		(TUTORIAL, "Ladder platform junction"))
	('Q', map_list_of
		(TUTORIAL, "(Glitched) Variable-length ladder spawn")
		(JUNGLE, "Variable-length vine spawn")
		(WORM, "Variable-length climbable uvula spawn")
		(ICE_CAVES, "(Glitched) Variable-length ladder spawn")
		(HELL, "Variable-length chain spawn"))
	('R', map_list_of
		(TUTORIAL, "Ruby tile"))
	('S', map_list_of
		(TUTORIAL, "Shop contents spawn"))
	('T', map_list_of
		(TUTORIAL, "Tree spawn")
		(ICE_CAVES, "Mothership Entrance")
		(TEMPLE, "Tree spawn"))
	('U', map_list_of
		(TUTORIAL, "Vlad spawn")
		(HAUNTED_CASTLE, "Black Knight spawn")
		(WORM, "Vlad spawn"))
	('V', map_list_of
		(TUTORIAL, "Inactive")
		(JUNGLE, "Vine generator (requires space)")
		(WORM, "Climbable uvula generator (requires space)")
		(ICE_CAVES, "Inactive")
		(HELL, "Brick + Enemy generator (requires space)"))
	('W', map_list_of
		(TUTORIAL, "Inactive outside shops"))
	('X', map_list_of
		(TUTORIAL, "Giant spider spawn")
		(JUNGLE, "Alien Lord spawn")
		(HELL, "Horsehead / Oxface spawn"))
	('Y', map_list_of
		(TUTORIAL, "Yeti King spawn")
		(TEMPLE, "Mummy spawn")
		(HELL, "King Yama spawn"))
	('Z', map_list_of
		(TUTORIAL, "Beehive terrain tile"))
	('a', map_list_of
		(TUTORIAL, "Pot spawn")
		(MINES, "Ankh spawn"))
	('b', map_list_of
		(TUTORIAL, "Shop floor tile"))
	('c', map_list_of
		(TUTORIAL, "Crystal skull spawn")
		(WORM, "Crysknife spawn (horizontally probabilistic)")
		(ICE_CAVES, "Crystal skull spawn"))
	('d', map_list_of
		(TUTORIAL, "Jungle terrain tile")
		(JUNGLE, "Wooden tile")
		(WORM, "Breakable meat block")
		(ICE_CAVES, "Jungle terrain tile"))
	('e', map_list_of
		(TUTORIAL, "Crate containing bombs")
		(MINES, "Beehive terrain tile"))
	('f', map_list_of
		(TUTORIAL, "Blue falling platform")
		(HELL, "Red falling platform"))
	('g', map_list_of
		(TUTORIAL, "Unlockable character / hired help coffin spawn"))
	('h', map_list_of
		(TUTORIAL, "Crate containing ropes")
		(MINES, "Hell terrain tile")
		(JUNGLE, "Gravestone tile")
		(HAUNTED_CASTLE, "Sacrifice altar")
		(WORM, "Hell terrain tile"))
	('i', map_list_of
		(TUTORIAL, "Ice block tile")
		(JUNGLE, "Tiki village fireplace")
		(WORM, "Ice block tile"))
	('j', map_list_of
		(TUTORIAL, "Prevents ice block tiles from becoming icicles by Yeti King"))
	('k', map_list_of
		(TUTORIAL, "Shop type sign tile"))
	('l', map_list_of
		(TUTORIAL, "Lantern tile"))
	('m', map_list_of
		(TUTORIAL, "Unbreakable terrain tile"))
	('n', map_list_of
		(TUTORIAL, "Probabilistic snake/terrain/empty tile")
		(HELL, "Probabilistic enemy/terrain/empty tile"))
	('o', map_list_of
		(TUTORIAL, "Rock spawn"))
	('p', map_list_of
		(TUTORIAL, "Inactive")
		(JUNGLE, "Inactive, crashes game")
		(WORM, "Inactive"))
	('q', map_list_of
		(TUTORIAL, "Unmodified anti-obstacle terrain tile")
		(HELL, "Unmodified anti-obstacle brick tile"))
	('r', map_list_of
		(TUTORIAL, "Probabilistic temple/terrain/push block tile"))
	('s', map_list_of
		(TUTORIAL, "Spikes tile"))
	('t', map_list_of
		(TUTORIAL, "Temple/castle tile"))
	('u', map_list_of
		(TUTORIAL, "Bat spawn")
		(MINES, "Vampire spawn"))
	('v', map_list_of
		(TUTORIAL, "Wooden tile"))
	('w', map_list_of
		(TUTORIAL, "Water tile")
		(WORM, "Acid pool tile")
		(ICE_CAVES, "Water tile")
		(TEMPLE, "Lava tile"))
	('x', map_list_of
		(TUTORIAL, "Sacrifice altar")
		(HAUNTED_CASTLE, "Wall torch")
		(WORM, "Sacrifice altar"))
	('y', map_list_of
		(TUTORIAL, "Block containing ruby"))
	('z', map_list_of
		(TUTORIAL, "Chest spawn")
		(MINES, "Probabilistic beehive/empty tile")
		(JUNGLE, "Probabilistic beehive/bee spawn/empty tile")
		(WORM, "Probabilistic beehive/empty tile"))
	('|', map_list_of
		(TUTORIAL, "Vault spawn tile"))
	('~', map_list_of
		(TUTORIAL, "Alien bounce trap"));

namespace Description {
	std::string ChunkDescription(Chunk* cnk) {
		AreaRenderMode mode = mode_from_chunk(cnk);

		//write area name if from internal, otherwise chunk identifier
		if(mode != AreaRenderMode::INVALID && cnk->get_name().find("LevelGen") == 0) {
			switch(mode) {
			case TUTORIAL:
				return "Tutorial";
			case MINES:
				return "Mines";
			case JUNGLE:
				return "Jungle";
			case HAUNTED_CASTLE:
				return "Haunted Castle";
			case WORM:
				return "Worm";
			case ICE_CAVES:
				return "Ice Caves";
			case MOTHERSHIP:
				return "Mothership";
			case TEMPLE:
				return "Temple";
			case HELL:
				return "Hell";
			default:
				return "Unknown";
			}
		}
		else {
			return cnk->get_name();
		}
	}

	std::string TileDescription(char tile, AreaRenderMode mode) {
		auto& areas = descriptions[tile];

		const std::string* desc = nullptr;
		for(unsigned p = TUTORIAL; p != INVALID; ++p) {
			AreaRenderMode arm = static_cast<AreaRenderMode>(p);
			
			//default to mines tiles
			if(arm == AreaRenderMode::INVALID)
				arm = AreaRenderMode::MINES;

			auto it = areas.find(arm);
			if(it != areas.end()) {
				desc = &it->second;
			}

			if(arm == mode) {
				break;
			}
		}

		if(desc != nullptr)
			return *desc;
		else
			return std::string("'") + tile + "'";
	}

	std::string TileDescription(char tile, Chunk* parent) {
		return TileDescription(tile, mode_from_chunk(parent));
	}
}