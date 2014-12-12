#include "tile_draw.h"

AreaRenderMode mode_from_name(const std::string& area) {
	if(area.find("Tut") == 0)
		return AreaRenderMode::TUTORIAL;
	else if(area.find("1-") == 0)
		return AreaRenderMode::MINES;
	else if(area.find("2-") == 0 || area == "Black Market") 
		return AreaRenderMode::JUNGLE;
	else if(area == "Haunted Castle")
		return AreaRenderMode::HAUNTED_CASTLE;
	else if(area.find("3-") == 0)
		return AreaRenderMode::ICE_CAVES;
	else if(area.find("4-") == 0 || area == "Olmec (4-4)")
		return AreaRenderMode::TEMPLE;
	else if(area.find("5-") == 0 || area == "Yama (5-4)")
		return AreaRenderMode::HELL;
	else if(area == "Worm")
		return AreaRenderMode::WORM;
	else
		return AreaRenderMode::MINES;
}

AreaRenderMode mode_from_chunk(Chunk* cnk) {
	const std::string& area = cnk->get_name();
	
	//default chunks
	if(area.find("LevelGen_TutorialCnk") == 0)
		return AreaRenderMode::TUTORIAL;
	else if(area.find("LevelGen_MinesCnk") == 0)
		return AreaRenderMode::MINES;
	else if(area.find("LevelGen_JungleGeneralCnk") == 0 || area.find("LevelGen_JungleBlackMarketCnk") == 0)
		return AreaRenderMode::JUNGLE;
	else if(area.find("LevelGen_JungleHauntedMansionCnk") == 0)
		return AreaRenderMode::HAUNTED_CASTLE;
	else if(area.find("LevelGen_IceCavesGeneralCnk") == 0)
		return AreaRenderMode::ICE_CAVES;
	else if(area.find("LevelGen_TempleCnk") == 0 || area.find("LevelGen_OlmecCnk") == 0)
		return AreaRenderMode::TEMPLE;
	else if(area.find("LevelGen_HellCnk") == 0)
		return AreaRenderMode::HELL;
	else if(area.find("LevelGen_WormCnk") == 0)
		return AreaRenderMode::WORM;
	
	//custom chunks
	if(area.find("Tutorial-") == 0)
		return AreaRenderMode::TUTORIAL;
	else if(area.find("Mines-") == 0)
		return AreaRenderMode::MINES;
	else if(area.find("Jungle-") == 0 || area.find("JungleBlackMarket-") == 0)
		return AreaRenderMode::JUNGLE;
	else if(area.find("JungleHauntedCastle-") == 0)
		return AreaRenderMode::HAUNTED_CASTLE;
	else if(area.find("IceCaves-") == 0)
		return AreaRenderMode::ICE_CAVES;
	else if(area.find("Temple-") == 0 || area.find("TempleOlmec-") == 0)
		return AreaRenderMode::TEMPLE;
	else if(area.find("Hell-") == 0)
		return AreaRenderMode::HELL;
	else if(area.find("Worm-") == 0)
		return AreaRenderMode::WORM;
	else
		return AreaRenderMode::MINES;
}

Fl_Color tile_color(char tile) {
	switch(tile) {
	case 'w': //water
		return 0xAAAAFF00;
	case 'e': //bee-hive inner
		return 0xFFFF2000;
	case 'z': //bee-hive outer
		return 0xFF905000;
	case '#': //tnt
		return 0xFF805000;
	case '?': //?
		return 0xFFFFFF00;
	case '4': //movable push block
		return 0x95857500;
	case 'C': //non-movable push block
		return 0x786D5400;
	case '*': //mace
		return 0x60505300;
	case 'd': //jungle terrain
		return 0xADFF2F00;
	case 'I': //idol
		return 0xCC990000;
	case '~': //bounce trap
		return 0x79DC7900;
	case 'v': //wooden platform (usually)
		return 0x8B451300;
	case 'G': //ladder (in jungle)
		return 0xCB873600;
	case 'g': //coffin spawn
		return 0x99004C00;
	case 'a': //ankh
		return 0xE2B22700;
	case '9': //static door
		return 0xAA00BD00;
	case '8': //tile-generating door
		return 0x50005900;
	case 'R': //ruby
		return 0xE3404A00;
	case '.': //guaranteed tile
		return 0x33339F00;
	case 's': //spikes
		return 0xD5A59500;
	default:
		return ((tile - '0')*0x23456721) & 0xFFFFFF00;
	}
}

void draw_tile(char tile, int x, int y, int w, int h, AreaRenderMode arm) {
	Fl_Color ctile = tile_color(tile);
	char str[] = {tile, 0};

	fl_draw_box(Fl_Boxtype::FL_FLAT_BOX, x, y, w, h, fl_darker(ctile));
	fl_draw_box(Fl_Boxtype::FL_FLAT_BOX, x, y, w-1, h-1, ctile);

	fl_color(0);
	fl_draw(str, x, y, w, h, FL_ALIGN_INSIDE);
}