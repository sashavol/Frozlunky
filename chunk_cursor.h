#pragma once

#include <set>
#include <functional>
#include <vector>
#include <map>
#include "tile_chunk.h"
#include "entity_spawn_layer.h"
#include "message_grid.h"

#define MESSAGE_ENTITY 53

struct cursor_store {
	std::map<std::pair<int, int>, std::pair<char, int>> tiles;
	std::map<MessageGrid::cc_pos, std::string> messages;
};

//works with constant-size chunks as a cursor
struct ChunkCursor {
private:
	typedef std::function<void(int dsx, int dsy, int dex, int dey)> pos_fn;

public:
	MessageGrid message_grid;

private:
	bool read_only;
	
	std::shared_ptr<EntitySpawnLayer> entity_layer;
	std::vector<pos_fn> pos_change_cb;

	std::vector<Chunk*> chunks;
	int tw;
	int width;
	int height;

	int sx;
	int sy;
	int ex;
	int ey;

public:
	char tile;
	
	ChunkCursor(std::shared_ptr<GameHooks> gh, const std::vector<Chunk*>& chunks, std::shared_ptr<EntitySpawnBuilder> esb, int tw, bool read_only=false);
	void pos_callback(pos_fn fn);
	
private:
	void tileref(int x, int y, std::function<void(Chunk* c, int cx, int cy)> ref);

	void entity_write(int entity, int x, int y, bool zero_ti_=true);
	int entity_get(int x, int y) const;

	void write(char tile, int x, int y, bool zero_et_=true);
	char get(int x, int y) const;

	void write_message(int x, int y, const std::string& msg);
	void erase_message(int x, int y);

public:
	int entity_get() const;
	void entity_put(int entity);

	char get() const;
	void put(char tile);

	std::string get_message();
	void put_message(const std::string& msg);
	void clear_messages();
	
	bool in_bounds() const;

	bool try_dx(int dx);
	bool try_dy(int dy);
	bool try_dex(int dx);
	bool try_dey(int dy);
	bool try_dsx(int dx);
	bool try_dsy(int dy);

	int rsx() const;
	int rsy() const;
	int rex() const;
	int rey() const;

	void s(int x, int y);
	void e(int x, int y);
	
	int cc_width();
	int cc_height();

	cursor_store encode();

private:
	typedef std::set<std::pair<int, int>> fill_history;

	bool check_dir(int x, int y, char target, int entity_target);
	void fill_recurse(int x, int y, fill_history& history, char tile, int entity, char target, int entity_target);
	
public:
	void fill(char tile);
	void entity_fill(int entity);

	void decode(const cursor_store& store, bool horiz_mirror=false);
};