#pragma once

#include "tile_chunk.h"
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Scrollbar.H>
#include <cmath>
#include <algorithm>

#define EDITOR_SEL_COUNT 180u

class EditorWidget : public Fl_Widget {
private:
	std::vector<Chunk*> chunks;
	std::vector<Chunk*> selected_chunks;
	Fl_Scrollbar* sidebar_scrollbar;

public:
	EditorWidget(int x, int y, int w, int h, Fl_Scrollbar* scrollbar, std::vector<Chunk*> chunks) : 
		Fl_Widget(x,y,w,h,""),
		chunks(chunks),
		selected_chunks(chunks.begin(), chunks.begin()+min(chunks.size(), EDITOR_SEL_COUNT)),
		sidebar_scrollbar(scrollbar)
	{}

private:
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

				fl_color(tile + 0x05050500);
				fl_draw(str, dp.first, dp.second, xu, yu, FL_ALIGN_INSIDE);
			}
		}
	}


	void draw_sidebar() {
		///TODO
	}

	//DEBUG, this will be replaced with the editing side of the widget
#define SCALE_MULT 3
	void draw_content() {
		int x = this->x(), y = this->y();

		int yi = (*(std::max_element(selected_chunks.begin(), selected_chunks.end(), 
			[](const Chunk* a, const Chunk* b) {
				return a->get_height() < b->get_height();
			}
		)))->get_height()*SCALE_MULT + 5;

		for(Chunk* c : selected_chunks) {
			int w = this->w() / selected_chunks.size() - 5;
			render_chunk(c, x, y, 100, 100);
			x += 105;
			if(x > this->w() + 100) { //TODO this is not very accurate, fix this check.
				x = this->x();
				y += 90;
			}
		}
	}

public:
	virtual void draw() override {
		draw_sidebar();
		draw_content();
	}
};