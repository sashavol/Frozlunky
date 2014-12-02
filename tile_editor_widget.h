#pragma once

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

//TODO sidebar on the right for selecting tile types, 
	//left click to select tile,
    //right click to place tile,
	//middle-click to fill
//TODO be able to hold mouse
//TODO verify jungle does not crash
//TODO hints bar for displaying useful information on hover, i.e. sidebar
//TODO fix worm UI
//TODO default templates for chunks

enum Direction {
	UP, LEFT, RIGHT, DOWN
};

struct ChunkEnv {
	Chunk* cnk;
	int cx;
	int cy;
	int lcx;
	int lcy;

	//std::vector<LockedTile> tile_locks;

	ChunkEnv(Chunk* cnk) : 
		cnk(cnk), 
		cx(-1), 
		cy(-1),
		lcx(-1),
		lcy(-1)
	{}

	bool in_bounds() {
		return cx >= 0 && cy >= 0 && cx < cnk->get_width() && cy < cnk->get_height();
	}

	//try moving cursor.x by dx
	bool try_dx(int dx) {
		int rx = cx + dx;
		if(rx >= 0 && rx < cnk->get_width()) {
			lcx = cx;
			cx = rx;
			return true;
		}
		return false;
	}

	//try moving cursor.y by dy
	bool try_dy(int dy) {
		int ry = cy + dy;
		if(ry >= 0 && ry < cnk->get_height()) {
			lcy = cy;
			cy = ry;
			return true;
		}
		return false;
	}

	bool locked(int x, int y) {
		/*for(auto&& locks : tile_locks) {
			if(locks.x == x && locks.y == y)
				return true;
		}*/
		return false;
	}

	//place tile at cursor
	void put(char tile) {
		//put lock will not be enforced, locks are soft.
		//if(locked(cx, cy))
		//	return;

		cnk->tile(cx, cy, tile);
	}
};

static Chunk* clipboard = nullptr;

#define STATE_CHUNK_APPLY (1 << 0)
#define STATE_CHUNK_WRITE (1 << 1)
#define STATE_CHUNK_COPY  (1 << 2)
#define STATE_CHUNK_PASTE (1 << 3)

class EditorWidget : public Fl_Widget {
public:
	int cnk_render_w, cnk_render_h;
	int hv_gap;
	Fl_Scrollbar* sidebar_scrollbar;

private:
	typedef std::chrono::high_resolution_clock clock;

	std::shared_ptr<StaticChunkPatch> tp;
	std::function<void(unsigned)> status_cb;

	ChunkTimeline timeline;
	TilePicker picker;

	std::vector<Chunk*> chunks;
	std::map<Chunk*, ChunkEnv*> envs;
	ChunkEnv* active_env;

	bool extended_mode; //extended mode (applicable to Worm, derived from chunks size)

	bool mouse_down[3];
	bool ctrl_down;
	bool shift_down;
	bool alt_down;
	std::shared_ptr<clock::time_point> key_press;

	bool allow_input() {
		if(!key_press)
			return true;

		return std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - *key_press).count() > 20;
	}

	void status(unsigned state) {
		if(status_cb)
			status_cb(state);
	}

//outside interface
public:
	std::vector<Chunk*> get_chunks() {
		return chunks;
	}

	void status_callback(std::function<void(unsigned)> cb) {
		this->status_cb = cb;
	}

private:
	void clear_chunk(Chunk* cnk) {
		if(cnk->type() == ChunkType::Single) {
			SingleChunk* sc = static_cast<SingleChunk*>(cnk);
			std::string d = sc->get_data();
			std::fill(d.begin(), d.end(), '0');
			sc->set_data(d);
		}
		else {
			for(Chunk* c : static_cast<GroupChunk*>(cnk)->get_chunks()) {
				clear_chunk(c);
			}
		}
	}

public:
	void clear_chunks() {
		for(Chunk* c : chunks) {
			clear_chunk(c);
		}
		timeline.push_state();
	}

private:
	Direction last_dir;

	//OPT issue: chunk render width/height scanning are unsafe assumptions for non-uniform chunk sizes (not applicable currently)
	void shift_env_left(int u) {
		if(!active_env)
			return;
		
		if(!active_env->try_dx(-u)) {
			int lcy = active_env->cy;
			auto rpos = get_chunk_render_pos(active_env->cnk);
			Chunk* cnk = find_chunk(rpos.first - cnk_render_w + 1, rpos.second);
			if(cnk) {
				active_env = envs[cnk];
				active_env->cy = lcy;
				active_env->cx = cnk->get_width() - 1;
			}
		}
		last_dir = Direction::LEFT;
	}

	void shift_env_right(int u) {
		if(!active_env)
			return;
		
		if(!active_env->try_dx(u)) {
			int lcy = active_env->cy;
			auto rpos = get_chunk_render_pos(active_env->cnk);
			Chunk* cnk = find_chunk(rpos.first + cnk_render_w, rpos.second);
			if(cnk) {
				active_env = envs[cnk];
				active_env->cy = lcy;
				active_env->cx = 0;
			}
		}
		last_dir = Direction::RIGHT;
	}

	void shift_env_up(int u) {
		if(!active_env)
			return;
		
		if(!active_env->try_dy(-u)) {
			int lcx = active_env->cx;
			auto rpos = get_chunk_render_pos(active_env->cnk);
			Chunk* cnk = find_chunk(rpos.first, rpos.second - cnk_render_h + 1); 
			if(cnk) {
				active_env = envs[cnk];
				active_env->cx = lcx;
				active_env->cy = cnk->get_height() - 1;
			}
		}
		last_dir = Direction::UP;
	}

	void shift_env_down(int u) {
		if(!active_env)
			return;
		
		if(!active_env->try_dy(u)) {
			int lcx = active_env->cx;
			auto rpos = get_chunk_render_pos(active_env->cnk);
			Chunk* cnk = find_chunk(rpos.first, rpos.second + cnk_render_h);
			if(cnk) {
				active_env = envs[cnk];
				active_env->cx = lcx;
				active_env->cy = 0;
			}
		}
		last_dir = Direction::DOWN;
	}

	void shift_env_last(int u) {
		switch(last_dir){
		case Direction::UP:
			shift_env_up(u);
			break;
		case Direction::DOWN:
			shift_env_down(u);
			break;
		case Direction::LEFT:
			shift_env_left(u);
			break;
		default:
		case Direction::RIGHT:
			shift_env_right(u);
			break;
		}
	}

private:
	int mouse_event_id() {
		switch(Fl::event_state()) {
		case FL_BUTTON1:
			return 0;
		case FL_BUTTON2:
			return 1;
		case FL_BUTTON3:
			return 2;
		default:
			return 0;
		}
	}

	std::pair<int, int> cursor_rpos;
	void cursor_move(int rx, int ry) {
		Chunk* c = find_chunk(rx, ry);
		if(c) {
			active_env = envs[c];
					
			auto spos = get_chunk_render_pos(c);
			active_env->cx = c->get_width() * (rx - spos.first) / cnk_render_w;
			active_env->cy = c->get_height() * (ry - spos.second) / cnk_render_h;
			picker.unselect();
		}
		else {
			active_env = nullptr;
		}

		char ptile = picker.tile(rx, ry);
		if(ptile != 0) {
			picker.select(ptile);
		}

		cursor_rpos.first = rx;
		cursor_rpos.second = ry;
	
		parent()->redraw();
	}

	char cursor_tile() {
		if(cursor_rpos.first >= 0 && cursor_rpos.second >= 0) {
			Chunk* c = find_chunk(cursor_rpos.first, cursor_rpos.second);
			if(c) {
				return c->tile(active_env->cx, active_env->cy);
			}
			
			char ptile = picker.tile(cursor_rpos.first, cursor_rpos.second);
			if(ptile != 0) {
				return ptile;
			}
		}

		return 0;
	}
	
	std::pair<int, int> last_build;
	void cursor_build(int rx, int ry, bool drag) {
		char tile = cursor_tile();
		auto affect = [=](int x, int y) {
			Chunk* c = find_chunk(x, y);
			if(c && tile != 0) {
				auto spos = get_chunk_render_pos(c);
				c->tile(c->get_width() * (x - spos.first) / cnk_render_w, c->get_height() * (y - spos.second) / cnk_render_h, tile);
			}
		};

		if(drag) {
			double vx = rx - last_build.first, vy = ry - last_build.second;
			double len = sqrt(vx*vx + vy*vy);
			vx /= len; vy /= len;
			
			for(double x = last_build.first, y = last_build.second; x <= rx && y <= ry; x += vx, y += vy) {
				affect((int)x, (int)y);
			}
		}
		else {
			affect(rx, ry);
		}

		last_build = std::pair<int, int>(rx, ry);
		parent()->redraw();
	}

public:
	virtual int handle(int evt) override {
		switch(evt) {
		case FL_PUSH: //mouse presed
			mouse_down[mouse_event_id()] = true;
			
			if(Fl::event_state() & FL_BUTTON1)
				cursor_move(Fl::event_x(), Fl::event_y());
			else if(Fl::event_state() & FL_BUTTON3)
				cursor_build(Fl::event_x(), Fl::event_y(), false);

			return 1;

		case FL_RELEASE: //mouse btn released
			mouse_down[mouse_event_id()] = false;
			return 1;

		case FL_DRAG:
			if(Fl::event_state() & FL_BUTTON1)
				cursor_move(Fl::event_x(), Fl::event_y());
			else if(Fl::event_state() & FL_BUTTON3)
				cursor_build(Fl::event_x(), Fl::event_y(), true);

			return 1;

		case 0xC: //key typed
			{
				int key = Fl::event_key();

				if(!allow_input())
					break;

				//key_pressed only if not flag key
				if(key != 65505 && key != 65507 && key != 65508 && key != 65513 && key != 65514) { 
					key_press = std::make_shared<clock::time_point>(clock::now());
				}

				switch(key) {
				case 65289: //tab
					{
						auto p = std::find(chunks.begin(), chunks.end(), active_env->cnk);
						if(shift_down) {
							if(p != chunks.begin()) {
								active_env = envs[*(p-1)];
								if(!active_env->in_bounds()) {
									active_env->cx = 0;
									active_env->cy = 0;
								}
								parent()->redraw();
							}
						}
						else {
							if(p+1 != chunks.end()) {
								active_env = envs[*(p+1)];
								if(!active_env->in_bounds()) {
									active_env->cx = 0;
									active_env->cy = 0;
								}
								parent()->redraw();
							}
						}
					}
					return 1;
					
				case 65361:
					shift_env_left(ctrl_down ? 2 : 1);
					parent()->redraw();
					return 1;

				case 65362: //up
					shift_env_up(ctrl_down ? 2 : 1);
					parent()->redraw();
					return 1;

				case 65363:
					shift_env_right(ctrl_down ? 2 : 1);
					parent()->redraw();
					return 1;

				case 65364: //down
					shift_env_down(ctrl_down ? 2 : 1);
					parent()->redraw();
					timeline.push_state();
					return 1;

				case 32:    //space
					active_env->put('0');
					if(!ctrl_down)
						shift_env_right(1);
					parent()->redraw();
					timeline.push_state();
					return 1;

				case 65288: //backspace
					active_env->put('0');
					if(!ctrl_down)
						shift_env_left(1);
					parent()->redraw();
					timeline.push_state();
					return 1;

				case 65505: //shift
					shift_down = true;
					return 1;

				case 65507: //ctrl
				case 65508:
					ctrl_down = true;
					return 1;

				case 65513: //alt
				case 65514:
					alt_down = true;
					return 1;

				case 65535: //delete
					active_env->put('0');
					parent()->redraw();
					timeline.push_state();
					return 1;

				case 115: //s
					if(ctrl_down) {
						tp->apply_chunks();
						status(STATE_CHUNK_APPLY);
						return 1;
					}

				case 122: //z: undo
					this->take_focus();
					if(ctrl_down) {
						timeline.rewind();
						parent()->redraw();
						return 1;
					}
					
				case 121: //y: redo
					this->take_focus();
					if(ctrl_down) {
						timeline.forward();
						parent()->redraw();
						return 1;
					}

				case 99: //c
					this->take_focus();
					if(ctrl_down && active_env) {
						clipboard = active_env->cnk;
						status(STATE_CHUNK_COPY);
						return 1;
					}
					
				case 110: //n
					this->take_focus();
					if(ctrl_down && active_env) {
						int w = active_env->cnk->get_width();
						int h = active_env->cnk->get_height();
						
						for(int y = 0; y < h; y++) {
							for(int x = 0; x < w; x++) {
								if(!active_env->locked(x, y))
									active_env->cnk->tile(x, y, '0');
							}
						}

						parent()->redraw();
						timeline.push_state();
						return 1;
					}

				case 118: //v
					this->take_focus();
					if(ctrl_down && active_env && clipboard) {
						if(clipboard->get_width() == active_env->cnk->get_width()
							&& clipboard->get_height() == active_env->cnk->get_height()) 
						{
							int w = clipboard->get_width(), h = clipboard->get_height();

							//horizontal mirror paste
							if(shift_down) {
								for(int y = 0; y < h; y++) {
									for(int x = 0; x < w; x++) {
										if(!active_env->locked(x, y))
											active_env->cnk->tile(x, y, clipboard->tile(w-x-1, y));
									}
								}
							}
							else {
								for(int y = 0; y < h; y++) {
									for(int x = 0; x < w; x++) {
										if(!active_env->locked(x, y))
											active_env->cnk->tile(x, y, clipboard->tile(x, y));
									}
								}
							}

							status(STATE_CHUNK_PASTE);
							parent()->redraw();
							return 1;
						}
						timeline.push_state();
					}

				default:
					{
						this->take_focus();
						char tile = Fl::event_text()[0];
						if(active_env && tp->valid_tile(tile)) {
							active_env->put(tile);
							if(alt_down)
								shift_env_last(1);
							else
								shift_env_right(1);
							status(STATE_CHUNK_WRITE);
							parent()->redraw();
						}
						timeline.push_state();
						return 1;
					}
				}
			}
			//key pressed
			return 1;

		case FL_KEYUP:
			key_press = nullptr;

			switch(Fl::event_key()) {
			case 65505:
				shift_down = false;
				return 1;
			case 65507:
			case 65508:
				ctrl_down = false;
				return 1;
			case 65513:
			case 65514:
				alt_down = false;
				return 1;
			}
			//key released
			return 1;

		case FL_FOCUS:
			return 1;
		case FL_UNFOCUS:
			return 1;
		}

		return Fl_Widget::handle(evt);
	}

public:
	~EditorWidget() {
		for(std::pair<Chunk*, ChunkEnv*> ev : envs) {
			delete ev.second;
		}
		envs.clear();
		active_env = nullptr;
	}

	EditorWidget(std::shared_ptr<StaticChunkPatch> tp, int x, int y, int w, int h, Fl_Scrollbar* scrollbar, std::vector<Chunk*> chunks, bool extended_mode=false) : 
		Fl_Widget(x,y,w,h,""),
		chunks(chunks),
		sidebar_scrollbar(scrollbar),
		cnk_render_w(130),
		cnk_render_h(104),
		hv_gap(0),
		active_env(nullptr),
		ctrl_down(false),
		shift_down(false),
		alt_down(false),
		tp(tp),
		extended_mode(extended_mode),
		last_dir(Direction::UP),
		timeline(chunks),
		cursor_rpos(-1, -1),
		picker(tp->valid_tiles(), x + w - 87, y, 45, h, 15, 15)
	{
		//allocate width for sidebar
		w -= 60;

		std::fill(mouse_down, mouse_down+sizeof(mouse_down), false);

		if(chunks.empty()) {
			this->deactivate();
			return;
		}

		for(Chunk* c : chunks) {
			envs[c] = new ChunkEnv(c);
		}

		double ratio = this->h() / (double)(get_chunk_render_pos(*chunks.rbegin()).second - this->y());
		if(ratio >= 1.0) {
			sidebar_scrollbar->deactivate();
			sidebar_scrollbar->slider_size(1.0);
		}
		else { 
			sidebar_scrollbar->activate();
			sidebar_scrollbar->slider_size(ratio);
			sidebar_scrollbar->type(FL_VERTICAL);
			sidebar_scrollbar->bounds(0, get_chunk_render_pos(*chunks.rbegin()).second);
			sidebar_scrollbar->step(10);

			sidebar_scrollbar->callback([](Fl_Widget* wid) {
				wid->redraw();
			});
		}
	}


private:
	Chunk* find_chunk(int rx, int ry) {
		int w = this->w();
		int x = this->x(), y = this->y();
		int hc = 0;

		ry += sidebar_scrollbar->value();

		for(Chunk* c : chunks) {
			if((rx >= x && rx < x + cnk_render_w)
			&& (ry >= y && ry < y + cnk_render_h))
			{
				return c;
			}

			hc++;
			x += cnk_render_w + hv_gap;
			if(hc == 4 || (extended_mode && hc == 2)) {
				x = this->x();
				y += cnk_render_h + hv_gap;
				hc = 0;
			}
		}

		return nullptr;
	}

	std::pair<int, int> get_chunk_render_pos(Chunk* cnk) {
		int w = this->w();
		int x = this->x();
		int y = this->y();

		int hc = 0;

		for(Chunk* c : chunks) {
			if(c == cnk)
				return std::pair<int, int>(x, y - sidebar_scrollbar->value());

			hc++;
			x += cnk_render_w + hv_gap;
			if(hc == 4 || (extended_mode && hc == 2)) { //TODO this is not very accurate, fix this check.
				x = this->x();
				y += cnk_render_h + hv_gap;
				hc = 0;
			}
		}

		throw std::runtime_error("Chunk from non-native editor.");
	}

	static void fit_chunk_aspect(Chunk* cnk, int& maxw, int& maxh) {
		int ch = cnk->get_height(), cw = cnk->get_width();
		if(cw < ch) {
			if(maxw < maxh)
				maxh = ch * maxw / cw;
			else
				maxw = cw * maxh / ch;
		}
		else {
			if(maxw < maxh)
				maxw = cw * maxh / ch;
			else
				maxh = ch * maxw / cw;
		}
	}

	void render_chunk(Chunk* cnk, int px, int py, int maxw, int maxh) {
		int ch = cnk->get_height(), cw = cnk->get_width();
		fit_chunk_aspect(cnk, maxw, maxh);

		int xu = maxw / cw, yu = maxh / ch;

		//maps chunk coordinates to world coordinates
		auto map = [&](int cx, int cy) -> std::pair<int, int> {
			return std::pair<int, int>(px + cx*maxw / cw, py + cy*maxh / ch); 
		};

		fl_font(FL_SCREEN, min(xu, yu)-2);
	
		for(int cy = 0; cy < ch; cy++) {
			for(int cx = 0; cx < cw; cx++) {
				char tile = cnk->tile(cx, cy);
				auto dp = map(cx, cy);
				draw_tile(tile, dp.first, dp.second, xu, yu);			
			}
		}

		picker.draw();
	}

	void render_env(int rx, int ry) {
		Chunk* c = active_env->cnk;
		fl_rect(rx, ry, cnk_render_w, cnk_render_h, 0x00006000);
		
		int maxw = cnk_render_w, maxh = cnk_render_h;
		fit_chunk_aspect(c, maxw, maxh);
		
		int xu = maxw/c->get_width(), yu = maxh/c->get_height();

		/*for(auto&& lock : active_env->tile_locks) {
			fl_rect(rx + xu*lock.x, ry + yu*lock.y, xu, yu, 0x60606000);
		}*/

		fl_rect(rx + xu*active_env->cx, ry + yu*active_env->cy, xu, yu, 0xFF000000);
	}

public:
	virtual void draw() override {
		int x = this->x(), y = this->y();
		int hc = 0;

		for(Chunk* c : chunks) {
			int rx = x, ry = y - sidebar_scrollbar->value();

			if(!(ry < -cnk_render_h || ry > this->h() + cnk_render_h)) {
				render_chunk(c, rx, ry, cnk_render_w, cnk_render_h);
			}
			
			hc++;
			x += cnk_render_w + hv_gap;
			if(hc == 4 || (extended_mode && hc == 2)) {
				x = this->x();
				y += cnk_render_h + hv_gap;
				hc = 0;
			}

			//active chunk
			if(active_env && active_env->cnk == c && active_env->in_bounds()) {
				render_env(rx, ry);
			}
		}
	}
};

struct EditorScrollbar : public Fl_Scrollbar {
	EditorWidget* editor;
	
	EditorScrollbar(int x, int y, int w, int h) : Fl_Scrollbar(x,y,w,h), editor(nullptr) {}

	void set_parent_editor(EditorWidget* editor) {
		this->editor = editor;
	}
	
	virtual int handle(int evt) override {
		if(evt == 5 || evt == 0x13) {
			auto par = parent();
			if(par)
				par->redraw();
		}
		else if(evt == 0xC || evt == 0x9)
			return 0;
		else if(evt == FL_FOCUS)
			return 0;
		
		return Fl_Scrollbar::handle(evt);
	}
};