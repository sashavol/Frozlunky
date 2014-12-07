#pragma once

#include "chunk_cursor.h"
#include "static_chunk_patch.h"
#include "chunk_timeline.h"
#include "tile_chunk.h"
#include "tile_draw.h"
#include "tile_picker.h"

#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl.H>

#include <cmath>
#include <map>
#include <algorithm>
#include <functional>
#include <chrono>

//OPT separate city of gold support
//OPT save force-level attribute to level exports for single-level challenges

//TODO add menu bar
//TODO move clear level button away from Force level, and move Force level checkbox up
//TODO fix worm / tutorial selection + UI
//TODO hints bar for displaying useful information on hover, i.e. sidebar
//TODO default templates for chunks

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
#define STATE_REQ_RANDOMIZE (1 << 7)
#define STATE_REQ_OPEN (1 << 8)
#define STATE_REQ_DEFAULT_SWAP (1 << 9)

class EditorWidget : public Fl_Widget {
public:
	bool read_only;

	AreaRenderMode arm;

	int cnk_render_w, cnk_render_h;
	int xu, yu;
	int hv_gap;
	Fl_Scrollbar* sidebar_scrollbar;

	bool mouse_down[3];
	bool ctrl_down;
	bool shift_down;
	bool alt_down;

private:
	typedef std::chrono::high_resolution_clock clock;

	std::shared_ptr<StaticChunkPatch> tp;
	std::function<void(unsigned)> status_cb;

	ChunkTimeline timeline;
	TilePicker picker;

	std::vector<Chunk*> chunks;
	ChunkCursor cursor;

	bool extended_mode; //extended mode (applicable to Worm, derived from chunks size)
	std::shared_ptr<clock::time_point> key_press;

	bool EditorWidget::allow_input();
	void status(unsigned state);

//outside interface
public:
	std::vector<Chunk*> get_chunks();
	void status_callback(std::function<void(unsigned)> cb);
	void clear_state(); //clear state (undos)

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

private:
	int mouse_event_id();

	std::pair<int, int> move_drag_start;
	void cursor_move(int rx, int ry, bool drag);
	void cursor_finish_move();
	char cursor_tile();
	void cursor_fill(int x, int y);
	
	std::pair<int, int> last_build;
	void cursor_build(int rx, int ry, bool drag);

	void compute_u();

public:
	virtual int handle(int evt) override;

public:
	~EditorWidget();
	EditorWidget(AreaRenderMode arm, std::shared_ptr<StaticChunkPatch> tp, int x, int y, int w, int h, Fl_Scrollbar* scrollbar, std::vector<Chunk*> chunks, bool extended_mode=false, bool read_only=false);


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