#pragma once

#include "tile_editor_widget.h"

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Input.h>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Button.H>
#include "tile_picker.h"

#include <iostream>
#include <iomanip>

namespace TilePickerSearch {
	class TileList : public Fl_Multi_Browser {
	private:
		int selected_y();

	public:
		TileList(int x, int y, int w, int h);

		void load_list(const std::vector<std::string>& tiles);
		void shift_selection(int dy);

		virtual int handle(int evt) override;
	};

	class TileSearch : public Fl_Input {
	private:
		std::string last_query;

	public:
		TileSearch(int x, int y, int w, int h);

		virtual int handle(int evt) override;
	};


	class PickButton : public Fl_Button {
	public:
		void do_pick();
	
		PickButton(int x, int y, int w, int h);

		virtual int handle(int evt) override;
	};

	class CancelButton : public Fl_Button {
	public:
		CancelButton(int x, int y, int w, int h);

		virtual int handle(int evt) override;
	};

	class SearchWindow : public Fl_Double_Window {
	private:
		EditorWidget* editor;
		TilePicker* picker;
		TileList* tile_list;
		TileSearch* search_input;
		PickButton* pick_button;
		CancelButton* cancel_button;

	public:
		SearchWindow(EditorWidget* editor);

		int handle_input_event(int evt);
		virtual int handle(int evt) override;

		friend class TilePicker;
		friend class TileList;
		friend class TileSearch;
		friend class PickButton;
		friend class CancelButton;
	};
}