#pragma once

#include "tile_chunk.h"
#include "tile_patch.h"
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl.H>
#include <cmath>
#include <map>
#include <algorithm>
#include <functional>
#include <chrono>

//OPT set up force chunk patch for particular functions, that could be useful.
//		this should probably be integrated into tile_patch.cpp somehow, since it maintains root chunk references

//TODO fix mouse no longer working after using tab / shift+tab

struct LockedTile {
	int x;
	int y;
	char value;

	LockedTile(int x, int y, char value) : x(x), y(y), value(value) {}
};

struct ChunkEnv {
	Chunk* cnk;
	int cx;
	int cy;
	int lcx;
	int lcy;

	std::vector<LockedTile> tile_locks;

	ChunkEnv(Chunk* cnk) : 
		cnk(cnk), 
		cx(-1), 
		cy(-1),
		lcx(-1),
		lcy(-1)
	{
		int w = cnk->get_width(), h = cnk->get_height();
		for(int y = 0; y < h; y++) {
			for(int x = 0; x < w; x++) {
				char tile = cnk->tile(x, y);
				if(tile == '9' || tile == '8') {
					tile_locks.push_back(LockedTile(x, y, tile));
					continue;
				}

				if(tile == '0' && (x == 0 || x == w-1 || y == 0 || y == h-1)) {
					tile_locks.push_back(LockedTile(x, y, tile));
					continue;
				}
			}
		}
	}

	bool in_bounds() {
		return cx >= 0 && cy >= 0 && cx < cnk->get_width() && cy < cnk->get_height();
	}

	//try moving cursor.x by dx
	void try_dx(int dx) {
		int rx = cx + dx;
		if(rx >= 0 && rx < cnk->get_width()) {
			lcx = cx;
			cx = rx;
		}
	}

	//try moving cursor.y by dy
	void try_dy(int dy) {
		int ry = cy + dy;
		if(ry >= 0 && ry < cnk->get_height()) {
			lcy = cy;
			cy = ry;
		}
	}

	bool locked(int x, int y) {
		for(auto&& locks : tile_locks) {
			if(locks.x == x && locks.y == y)
				return true;
		}
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

	std::shared_ptr<TilePatch> tp;
	std::function<void(unsigned)> status_cb;

	std::vector<Chunk*> chunks;
	std::map<Chunk*, ChunkEnv*> envs;
	ChunkEnv* active_env;

	bool ctrl_down;
	bool shift_down;
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

public:
	void status_callback(std::function<void(unsigned)> cb) {
		this->status_cb = cb;
	}

	virtual int handle(int evt) override {
		//TODO handle swapping between chunkenvs, editing chunks using envs, etc.
		switch(evt) {
		case 0x1:
			if(Fl::event_state() == FL_BUTTON1) {
				Chunk* c = find_chunk(Fl::event_x(), Fl::event_y());
				if(c) {
					active_env = envs[c];
					
					auto spos = get_chunk_render_pos(c);
					active_env->cx = c->get_width() * (Fl::event_x() - spos.first) / cnk_render_w;
					active_env->cy = c->get_height() * (Fl::event_y() - spos.second) / cnk_render_h;

					parent()->redraw();
				}
			}
			this->take_focus();
			break;
		case 0x2:
			//mouse btn released
			break;

		case 0xC:
			{
				int key = Fl::event_key();

				if(!allow_input())
					break;

				//key_pressed only if not flag key
				if(key != 65505 && key != 65507 && key != 65508) { 
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
					break;

				case 65361:
					active_env->try_dx(ctrl_down ? -2 : -1);
					parent()->redraw();
					break;

				case 65362: //up
					active_env->try_dy(ctrl_down ? -2 : -1);
					parent()->redraw();
					break;

				case 65363:
					active_env->try_dx(ctrl_down ? 2 : 1);
					parent()->redraw();
					break;

				case 65364: //down
					active_env->try_dy(ctrl_down ? 2 : 1);
					parent()->redraw();
					break;

				case 32:    //space
					active_env->put('0');
					if(!ctrl_down)
						active_env->try_dx(1);
					parent()->redraw();
					break;

				case 65288: //backspace
					active_env->put('0');
					if(!ctrl_down)
						active_env->try_dx(-1);
					parent()->redraw();
					break;

				case 65505: //shift
					shift_down = true;
					break;

				case 65507: //ctrl
				case 65508:
					ctrl_down = true;
					break;

				case 65535: //delete
					active_env->put('0');
					parent()->redraw();
					break;

				case 115: //s
					if(ctrl_down) {
						tp->apply_chunks();
						status(STATE_CHUNK_APPLY);
						break;
					}

				case 99: //c
					if(ctrl_down && active_env) {
						clipboard = active_env->cnk;
						status(STATE_CHUNK_COPY);
						break;
					}
					
				case 110: //n
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
						break;
					}

				case 118: //v
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
							break;
						}
					}

				default:
					{
						char tile = Fl::event_text()[0];
						if(tp->valid_tile(tile)) {
							active_env->put(tile);
							if(!ctrl_down) {
								active_env->try_dx(1);
							}
							status(STATE_CHUNK_WRITE);
							parent()->redraw();
						}
					}
				}
			}
			//key pressed
			break;
		case 0x9:
			key_press = nullptr;

			switch(Fl::event_key()) {
			case 65505:
				shift_down = false;
				break;
			case 65507:
			case 65508:
				ctrl_down = false;
				break;
			}
			//key released
			break;

		case FL_FOCUS:
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

	EditorWidget(std::shared_ptr<TilePatch> tp, int x, int y, int w, int h, Fl_Scrollbar* scrollbar, std::vector<Chunk*> chunks) : 
		Fl_Widget(x,y,w,h,""),
		chunks(chunks),
		sidebar_scrollbar(scrollbar),
		cnk_render_w(100),
		cnk_render_h(85),
		hv_gap(5),
		active_env(nullptr),
		ctrl_down(false),
		shift_down(false),
		tp(tp)
	{
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
		
		ry += sidebar_scrollbar->value();

		for(Chunk* c : chunks) {
			if((rx >= x && rx < x + cnk_render_w)
			&& (ry >= y && ry < y + cnk_render_h))
			{
				return c;
			}

			x += cnk_render_w + hv_gap;
			if(x > w + cnk_render_w) {
				x = this->x();
				y += cnk_render_h + hv_gap;
			}
		}

		return nullptr;
	}

	std::pair<int, int> get_chunk_render_pos(Chunk* cnk) {
		int w = this->w();
		int x = this->x();
		int y = this->y();

		for(Chunk* c : chunks) {
			if(c == cnk)
				return std::pair<int, int>(x, y - sidebar_scrollbar->value());

			x += cnk_render_w + hv_gap;
			if(x > w + cnk_render_w) { //TODO this is not very accurate, fix this check.
				x = this->x();
				y += cnk_render_h + hv_gap;
			}
		}

		throw std::runtime_error("Chunk from non-native editor.");
	}

	static inline Fl_Color tile_color(const Chunk* cnk, char tile) {
		switch(tile) {
		case 'w': //water
			return 0xAAAAFF00;
		case 'e': //bee-hive inner
			return 0xFFFF2000;
		case 'z': //bee-hive outer
			return 0xFF905000;
		default:
			return ((tile - '0')*0x23456721) & 0xFFFFFF00;
		}
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
				Fl_Color ctile = tile_color(cnk, tile);
				auto dp = map(cx, cy);
				char str[] = {tile, 0};
				
				fl_draw_box(Fl_Boxtype::FL_FLAT_BOX, dp.first, dp.second, xu, yu, fl_darker(ctile));
				fl_draw_box(Fl_Boxtype::FL_FLAT_BOX, dp.first, dp.second, xu-1, yu-1, ctile);

				fl_color(0);
				fl_draw(str, dp.first, dp.second, xu, yu, FL_ALIGN_INSIDE);
			}
		}
	}

	void render_env(int rx, int ry) {
		Chunk* c = active_env->cnk;
		fl_rect(rx, ry, cnk_render_w, cnk_render_h, 0x0000FF00);
		
		int maxw = cnk_render_w, maxh = cnk_render_h;
		fit_chunk_aspect(c, maxw, maxh);
		
		int xu = maxw/c->get_width(), yu = maxh/c->get_height();

		for(auto&& lock : active_env->tile_locks) {
			fl_rect(rx + xu*lock.x, ry + yu*lock.y, xu, yu, 0x60606000);
		}

		fl_rect(rx + xu*active_env->cx, ry + yu*active_env->cy, xu, yu, 0xFF000000);
	}

public:
	virtual void draw() override {
		int x = this->x(), y = this->y();

		for(Chunk* c : chunks) {
			int rx = x, ry = y - sidebar_scrollbar->value();

			if(!(ry < -cnk_render_h || ry > this->h() + cnk_render_h)) {
				render_chunk(c, rx, ry, cnk_render_w, cnk_render_h);
			}
			
			x += cnk_render_w + hv_gap;
			if(x > this->w() + cnk_render_w) {
				x = this->x();
				y += cnk_render_h + hv_gap;
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