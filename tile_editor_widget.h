#pragma once

#include "chunk_cursor.h"
#include "static_chunk_patch.h"
#include "chunk_timeline.h"
#include "tile_chunk.h"
#include "tile_draw.h"
#include "tile_picker.h"
#include "tile_editing_hintbar.h"
#include "entity_spawn_layer.h"

#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl.H>

#include <cmath>
#include <map>
#include <algorithm>
#include <functional>
#include <chrono>
#include <atomic>


//OPT shops
//OPT separate city of gold support
//OPT save force-level attribute to level exports for single-level challenges
//OPT boshy-style level checkpoints through automated force level

enum Direction {
	UP, LEFT, RIGHT, DOWN
};

#define STATE_CHUNK_APPLY (1 << 0)
#define STATE_CHUNK_WRITE (1 << 1)
#define STATE_CHUNK_COPY  (1 << 2)
#define STATE_CHUNK_PASTE (1 << 3)
#define STATE_RESERVED1   (1 << 4)
#define STATE_REQ_TAB (1 << 5)
#define STATE_REQ_TAB_REVERSE (1 << 6)
#define STATE_REQ_OPEN (1 << 8)
#define STATE_REQ_DEFAULT_SWAP (1 << 9)
#define STATE_REQ_SAVE	       (1 << 10)
#define STATE_REQ_SAVE_AS	   (1 << 11)
#define STATE_REQ_NEW_FILE	   (1 << 12)
#define STATE_REQ_TOGGLE_FORCE_LEVEL (1 << 13)
#define STATE_REQ_RESOURCE_EDITOR (1 << 14)
#define STATE_REQ_ENTITY_PICKER (1 << 15)
#define STATE_REQ_RESIZE_1 (1 << 16)
#define STATE_REQ_RESIZE_2 (1 << 17)
#define STATE_REQ_RESIZE_3 (1 << 18)
#define STATE_REQ_RESIZE_4 (1<< 19)
#define STATE_REQ_RESIZE_1_5 (1 << 20)
#define STATE_REQ_LEVEL_SETTINGS (1 << 21)
#define STATE_REQ_TILE_SEARCH (1 << 22)

class EditorWidget : public Fl_Widget {
public:
	std::shared_ptr<bool> dark_status;
	std::atomic<bool> disable_ghost;

public:
	bool read_only;

	AreaRenderMode arm;

	int last_w, last_h;
	int cnk_render_w, cnk_render_h;
	int xu, yu;
	
	bool use_scrollbar;
	Fl_Scrollbar* sidebar_scrollbar;
	TileEditingHintbar* hint_bar;

	bool mouse_down[3];
	bool ctrl_down;
	bool shift_down;
	bool alt_down;

private:
	typedef std::chrono::high_resolution_clock clock;

	std::shared_ptr<EntitySpawnBuilder> esb;
	std::shared_ptr<StaticChunkPatch> tp;
	std::function<void(unsigned)> status_cb;

	TilePicker picker;

	std::vector<Chunk*> chunks;
	ChunkCursor cursor;

	ChunkTimeline timeline;

	bool extended_mode; //extended mode (applicable to Worm, derived from chunks size)
	std::shared_ptr<clock::time_point> key_press;

	bool EditorWidget::allow_input();
	void status(unsigned state);
	void update_hint_bar();

//outside interface
public:
	std::shared_ptr<EntitySpawnBuilder> get_entity_builder();
	std::vector<Chunk*> get_chunks();
	void status_callback(std::function<void(unsigned)> cb);
	void clear_state(); //clear state (undos)
	ChunkCursor& get_cursor();
	TilePicker& get_picker();

private:
	void clear_chunk(Chunk* cnk);
public:
	void clear_chunks();

private:
	Direction last_dir;

	//OPT issue: chunk render width/height scanning are unsafe assumptions for non-uniform chunk sizes (not applicable currently)
	void shift_env_left(int u, bool bc=false);
	void shift_env_right(int u, bool bc=false);
	void shift_env_up(int u, bool bc=false);
	void shift_env_down(int u, bool bc=false);
	void shift_env_last(int u);

	void shift_picker_cursor(int dx, int dy);

private:
	int mouse_event_id();

	std::pair<int, int> move_drag_start;
	void cursor_move(int rx, int ry, bool drag);
	void cursor_finish_move();

	void cursor_fill(int x, int y);
	
	std::pair<int, int> last_build;
	std::pair<int, int> build_dim;
	void cursor_build(int rx, int ry, bool drag, char default_tile = 0);

	void ensure_size();

public:
	//throws std::invalid_argument if key does nothing.
	int handle_key(int key);
	virtual int handle(int evt) override;

public:
	~EditorWidget();
	EditorWidget(AreaRenderMode arm, 
			std::shared_ptr<StaticChunkPatch> tp, 
			std::shared_ptr<EntitySpawnBuilder> esb, 
			int x, int y, int w, int h, 
			Fl_Scrollbar* scrollbar, 
			TileEditingHintbar* hint_bar, 
			std::vector<Chunk*> chunks, 
			bool extended_mode=false, 
			bool read_only=false);


private:
	Chunk* find_chunk(int rx, int ry);
	std::pair<int, int> get_chunk_render_pos(Chunk* cnk);
	std::pair<int, int> render_pos(int x, int y);
	std::pair<int, int> chunkcoord_pos(int rx, int ry);
	std::pair<int, int> chunkcoord_pos(Chunk* cnk);

private:
	static void fit_chunk_aspect(Chunk* cnk, int& maxw, int& maxh);

private:
	void render_chunk(Chunk* cnk, int px, int py, int maxw, int maxh);
	void render_cursor();

public:
	virtual void draw() override;
};

struct EditorScrollbar : public Fl_Scrollbar {
	EditorWidget* editor;
	
	EditorScrollbar(int x, int y, int w, int h);
	void set_parent_editor(EditorWidget* editor);
	
	virtual int handle(int evt) override;
};