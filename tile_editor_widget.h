#pragma once

#include "tile_chunk.h"
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Scrollbar.H>
#include <cmath>
#include <map>
#include <algorithm>

//OPT set up force chunk patch for particular functions, that could be useful.
//		this should probably be integrated into tile_patch.cpp somehow, since it maintains root chunk references

struct ChunkEnv {
	Chunk* cnk;
	int cx;
	int cy;

	ChunkEnv(Chunk* cnk) : 
		cnk(cnk), 
		cx(0), 
		cy(0) 
	{}
};

class EditorWidget : public Fl_Widget {
public:
	int cnk_render_w, cnk_render_h;
	int hv_gap;
	Fl_Scrollbar* sidebar_scrollbar;

private:
	std::vector<Chunk*> chunks;
	std::map<Chunk*, ChunkEnv*> envs;
	ChunkEnv* active_env;

public:
	virtual int handle(int evt) override {
		//TODO handle swapping between chunkenvs, editing chunks using envs, etc.

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

	EditorWidget(int x, int y, int w, int h, Fl_Scrollbar* scrollbar, std::vector<Chunk*> chunks) : 
		Fl_Widget(x,y,w,h,""),
		chunks(chunks),
		sidebar_scrollbar(scrollbar),
		cnk_render_w(100),
		cnk_render_h(85),
		hv_gap(5),
		active_env(nullptr)
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
			sidebar_scrollbar->bounds(0, this->h() + cnk_render_h*2);
			sidebar_scrollbar->step(10);

			sidebar_scrollbar->callback([](Fl_Widget* wid) {
				wid->redraw();
			});
		}
	}


private:
	std::pair<int, int> get_chunk_render_pos(Chunk* cnk) {
		int x = this->x();
		int y = this->y();

		for(Chunk* c : chunks) {
			if(c == cnk)
				return std::pair<int, int>(x, y);

			x += cnk_render_w + hv_gap;
			if(x > this->w() + cnk_render_w) { //TODO this is not very accurate, fix this check.
				x = this->x();
				y += cnk_render_h + hv_gap;
			}
		}

		throw std::runtime_error("Chunk from non-native editor.");
	}

	static inline Fl_Color tile_color(const Chunk* cnk, char tile) {
		return ((tile - '0')*0x23456721) & 0xFFFFFF00;
	}

	static void render_chunk(Chunk* cnk, int px, int py, int maxw, int maxh) {
		int ch = cnk->get_height(), cw = cnk->get_width();
		
		//correct aspect ratio
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
		if(evt == 5) {
			parent()->redraw();
		}
		return Fl_Scrollbar::handle(evt);
	}
};