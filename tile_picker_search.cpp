#include "tile_picker_search.h"
#include "tile_editor_widget.h"
#include "tile_description.h"
#include "known_entities.h"
#include "tile_util.h"

#include <map>

namespace TilePickerSearch {
	static std::map<std::string, char> fast_tile_lookup;
	
	static std::vector<std::string> Tiles(TilePicker* picker, AreaRenderMode arm, bool entities) {
		std::vector<std::string> out;

		for(char tile : picker->get_tiles()) {
			std::string desc = Description::TileDescription(tile, arm);
			if(fast_tile_lookup.find(desc) == fast_tile_lookup.end()) {
				fast_tile_lookup[desc] = tile;
			}
			out.push_back(desc);
		}

		if(entities) {
			for(int entity : picker->get_recent_entities()) {
				out.push_back(KnownEntities::GetName(entity));
			}
		}

		return out;
	}

	static std::vector<std::string> SearchTiles(TilePicker* picker, AreaRenderMode arm, bool entities, const std::string& query) {
		return TileUtil::Search(Tiles(picker, arm, entities), query);
	}
	
	static int GetEntity(const std::string& name) {
		return KnownEntities::GetID(name);
	}

	static char GetTile(const std::string& name) {
		return fast_tile_lookup[name];
	}

	TileList::TileList(int x, int y, int w, int h) : Fl_Multi_Browser(x, y, w, h) {
		SearchWindow* wp = static_cast<SearchWindow*>(parent());
		for(const std::string& et : Tiles(wp->picker, wp->editor->arm, !!wp->editor->get_entity_builder())) {
			this->add(et.c_str());
		}
	}

	void TileList::load_list(const std::vector<std::string>& tiles) {
		clear();
		for(const std::string& et : tiles) {
			add(et.c_str());	
		}

		if(tiles.size() > 0) {
			value(1);
		}
	}

	//computes the minimum required y visibility
	int TileList::selected_y() {
		if(value() == 0)
			return 0;
		else
			return (value()-1)*incr_height();
	}

	void TileList::shift_selection(int dy) {
		if(size() == 0)
			return;
	
		int idx = value();

		if(idx == 0) {
			value(1);
		}
		else {
			if(idx+dy < 1)
				dy = 1-idx;
			else if(idx > size()) 
				dy = size()-idx;

			idx += dy;

			if(idx-dy >= 1 && idx-dy <= size()) {
				select(idx-dy, 0);
			}
			value(idx);
		}

		int sel_y = selected_y();
		int curr_top = this->scrollbar.value();
		int curr_bottom = curr_top + this->h() - this->incr_height();

		if(!(sel_y >= curr_top && sel_y <= curr_bottom)) {
			if(dy > 0)
				this->topline(value());
			else
				this->bottomline(value());
		}
	}

	int TileList::handle(int evt) {
		//reject focus, parent window manages event dispatch
		if(evt == FL_FOCUS)
			return 0;
		else if(evt == FL_UNFOCUS)
			return 1;
		else if(evt == FL_KEYBOARD || evt == 0xC)
			return 0;

		return Fl_Multi_Browser::handle(evt);
	}

	/////////

	TileSearch::TileSearch(int x, int y, int w, int h) : Fl_Input(x, y, w, h, "Search:") {}

	int TileSearch::handle(int evt) {
		SearchWindow* wp = static_cast<SearchWindow*>(parent());

		//reject focus, parent window manages event dispatch
		if(evt == FL_FOCUS)
			return 0;
		else if(evt == FL_UNFOCUS)
			return 1;
		else if(evt == FL_PUSH || evt == FL_RELEASE || evt == FL_MOUSEWHEEL)
			return 0;

		int ret = Fl_Input::handle(evt);

		if(evt == FL_KEYBOARD || evt == 0xC) {
			int key = Fl::event_key();
			if(key == 65307 || key == 65362 || key == 65364 || key == 65293)
				return 0;

			std::string sval(value());
			if(sval != last_query) {
				TileList* elist = wp->tile_list;

				if(sval == "") {
					std::vector<std::string> all;
					for(const std::string& et : Tiles(wp->picker, wp->editor->arm, !!wp->editor->get_entity_builder())) {
						all.push_back(et);
					}

					elist->load_list(all);
				}
				else {
					elist->load_list(SearchTiles(wp->picker, wp->editor->arm, !!wp->editor->get_entity_builder(), sval));
				}

				last_query = sval;
			}
		}

		return ret;
	}

	//////////

	void PickButton::do_pick() {
		SearchWindow* ep = static_cast<SearchWindow*>(parent());
		TileList* list = ep->tile_list;
		const char* text = list->text(list->value());
		if(!text) {
			MessageBox(NULL, "No entity type is selected for picking.", "Tile Picker", MB_OK);
			return;
		}

		char tile = GetTile(text);
		int entity = GetEntity(text);

		if(!tile && !entity) {
			MessageBox(NULL, "Failed to find ID for the selected tile, please re-select and try again.", "Tile Picker", MB_OK);
			return;
		}

		if(entity) {
			ep->picker->entity_select(entity);
			ep->editor->hint_bar->set_entity(entity, nullptr);
		}
		else if(tile) {
			ep->picker->select(tile);
			ep->editor->hint_bar->set_tile(tile, ep->editor->arm, nullptr);
		}

		ep->hide();
		ep->editor->hint_bar->redraw();
		ep->editor->parent()->redraw();
		ep->editor->parent()->take_focus();
	}

	PickButton::PickButton(int x, int y, int w, int h) : Fl_Button(x, y, w, h, "Pick Tile") {
		callback([](Fl_Widget* w) {
			static_cast<PickButton*>(w)->do_pick();
		});
	}

	int PickButton::handle(int evt) {
		//reject focus, parent window manages event dispatch
		if(evt == FL_FOCUS)
			return 0;

		return Fl_Button::handle(evt);
	}

	//////////

	CancelButton::CancelButton(int x, int y, int w, int h) : Fl_Button(x, y, w, h, "Cancel") {
		callback([](Fl_Widget* w) {
			w->parent()->hide();
		});
	}

	int CancelButton::handle(int evt) {
		//reject focus, parent window manages event dispatch
		if(evt == FL_FOCUS)
			return 0;

		return Fl_Button::handle(evt);
	}

	///////////

	SearchWindow::SearchWindow(EditorWidget* editor) :
		Fl_Double_Window(303, 258, "Tile Picker Search"),
		editor(editor),
		picker(&editor->get_picker())
	{
		this->begin();
		{ tile_list = new TileList(10, 10, 285, 175);
		} // Fl_Browser* o
		{ search_input = new TileSearch(65, 191, 230, 27);
		} // Fl_Input* o
		{ pick_button = new PickButton(11, 225, 159, 25);
		} // Fl_Button* o
		{ cancel_button = new CancelButton(175, 225, 119, 25);
		} // Fl_Button* o
		this->end();
	}

	int SearchWindow::handle_input_event(int evt) {
		int key = Fl::event_key();
		bool alt_down = !!Fl::event_alt();
		bool shift_down = !!Fl::event_shift();
		bool ctrl_down = !!Fl::event_ctrl();

		if(evt == 0xC) {
			switch(key) {
			case 65307: //esc: close window
				this->hide();
				return 1;

			case 65362: //(ctrl+) up: move selection up
				tile_list->shift_selection(ctrl_down ? -2 : -1);
				return 1;

			case 65364: //(ctrl+) down: move selection down
				tile_list->shift_selection(ctrl_down ? 2 : 1);
				return 1;

			case 65293: //ENTER
				pick_button->do_pick();
				return 1;

			default:
				return search_input->handle(evt);
			}
		}
		else {
			return search_input->handle(evt);
		}
	}

	static bool cursor_event_bounding(Fl_Widget* w) {
		return Fl::event_x() >= w->x() 
			&& Fl::event_x() < w->x() + w->w() 
			&& Fl::event_y() >= w->y()
			&& Fl::event_y() < w->y()+w->h();
	}

	int SearchWindow::handle(int evt) {
		switch(evt) {
		case FL_SHOW:
			this->take_focus();
			return 1;

		case FL_FOCUS:
			return 1;
		
		case FL_UNFOCUS:
			return 0;

		case FL_PUSH:
		case FL_RELEASE:
			{
				if(cursor_event_bounding(tile_list)) {
					tile_list->handle(evt);
					take_focus();
					return 1;
				}
				else if(cursor_event_bounding(pick_button)) {
					pick_button->handle(evt);
					take_focus();
					return 1;
				}
				else if(cursor_event_bounding(cancel_button)) {
					cancel_button->handle(evt);
					take_focus();
					return 1;
				}
			}
		
		case FL_KEYBOARD:
		case 0xC: 
			return handle_input_event(evt);
		}

		return Fl_Double_Window::handle(evt);
	}
}