#include "level_forcer.h"

LevelForcer::~LevelForcer() {
	this->set_enabled(false);
}

LevelForcer::LevelForcer(std::shared_ptr<DerandomizePatch> dp, std::shared_ptr<GameHooks> gh) :
	spel(dp->spel),
	dp(dp),
	gh(gh),
	active(false),
	is_valid(true),
	flags(0),
	lvl(0)
{}

bool LevelForcer::enabled() {
	return active;
}

void LevelForcer::create_worker() {
	worker_thread = std::thread([=]() {
		while(active) {
			int state = gh->game_state();
			if(state == STATE_INPUTLOCK_CIRCLECLOSE || state == STATE_INPUTLOCK_GENERIC || state == STATE_INPUTLOCK_LEVELSTART || state == STATE_PLAYING)
			{
				Address game;
				spel->read_mem(dp->game_ptr(), &game, sizeof(Address));

				if(lvl > 0) {
					int level = lvl;
					spel->write_mem(game + dp->current_level_offset(), &level, sizeof(int));
				}

				unsigned fl = flags;
				const BYTE one = 1;
				if(fl & LF_BLACK_MARKET)
					spel->write_mem(game + gh->blackmkt_offset(), &one, sizeof(BYTE));
				if(fl & LF_CITY_OF_GOLD)
					spel->write_mem(game + gh->cog_offset(), &one, sizeof(BYTE));
				if(fl & LF_HAUNTED_MANSION)
					spel->write_mem(game + gh->haunted_mansion_offset(), &one, sizeof(BYTE));
				if(fl & LF_WORM)
					spel->write_mem(game + gh->worm_offset(), &one, sizeof(BYTE));
				if(fl & LF_YETI)
					spel->write_mem(game + gh->yeti_offset(), &one, sizeof(BYTE));
				if(fl & LF_MOTHERSHIP)
					spel->write_mem(game + gh->mothership_offset(), &one, sizeof(BYTE));
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
	});
}

void LevelForcer::force(int lvl, unsigned flags) {
	this->lvl = lvl;
	this->flags = flags;
}

void LevelForcer::set_enabled(bool enabled) { 
	if(active != enabled) {
		if(enabled) {
			active = true;
			create_worker();
		}
		else {
			active = false;
			worker_thread.join();
		}
	}
}

bool LevelForcer::valid() {
	return is_valid;
}