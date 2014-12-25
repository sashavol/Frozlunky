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

void LevelForcer::cycle() {
	if(!active) {
		return;
	}
	
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
}

void LevelForcer::force(int lvl, unsigned flags) {
	this->lvl = lvl;
	this->flags = flags;
}

void LevelForcer::set_enabled(bool enabled) { 
	active = enabled;
}

bool LevelForcer::valid() {
	return is_valid;
}

/////////
// level redirect
/////////

LevelRedirect::LevelRedirect(std::shared_ptr<GameHooks> gh) : 
	gh(gh),
	spel(gh->spel),
	level_start(LB_DEFAULT_START),
	level_olmec(LB_DEFAULT_OLMEC),
	level_yama(LB_DEFAULT_YAMA),
	last_checkpoint(LB_DEFAULT_START),
	checkpoint_mode(false),
	last_state(-1)
{}

void LevelRedirect::write_level(int target) {
	Address game;
	spel->read_mem(gh->dp->game_ptr(), &game, sizeof(Address));
	spel->write_mem(game + gh->dp->current_level_offset(), &target, sizeof(int));
}

void LevelRedirect::cycle() {
	int state = gh->game_state();
	int lvl = gh->current_level();

	if(game_started && (state == STATE_LOBBY || state == STATE_GAMEOVER_HUD || state == STATE_MAINMENU)) {
		game_started = false;
	}
	
	if(!game_started && (state == STATE_PLAYING || state == STATE_INPUTLOCK_LEVELSTART)) {
		game_started = true;
	}

	checkpoint_mutex.lock();
	if(!checkpoint_mode || state == STATE_LOBBY || state == STATE_MAINMENU) {
		last_checkpoint = int(level_start);
	}
	else if(state == STATE_PLAYING || state == STATE_INPUTLOCK_LEVELSTART) {
		last_checkpoint = lvl;
	}

	if(!game_started && (checkpoint_mode || level_start != LB_DEFAULT_START)) {
		write_level(last_checkpoint);
	}
	checkpoint_mutex.unlock();

	if(level_olmec != LB_DEFAULT_OLMEC) {
		if(lvl == level_olmec-1 && state == STATE_LEVEL_TRANSITION) {
			write_level(LB_DEFAULT_OLMEC-1);
		}
	}

	if(level_yama != LB_DEFAULT_YAMA) {
		if(lvl == level_yama-1 && state == STATE_LEVEL_TRANSITION) {
			write_level(LB_DEFAULT_YAMA-1);
		}
	}
	
	last_state = state;
}

bool LevelRedirect::valid() {
	return true;
}

void LevelRedirect::reset() {
	level_start = LB_DEFAULT_START;
	level_olmec = LB_DEFAULT_OLMEC;
	level_yama = LB_DEFAULT_YAMA;
	checkpoint_mode = false;

	checkpoint_mutex.lock();
	last_checkpoint = LB_DEFAULT_START;
	checkpoint_mutex.unlock();
}