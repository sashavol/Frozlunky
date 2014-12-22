#include "entity_picker.h"
#include "tile_editor_widget.h"

EntityList::EntityList(int x, int y, int w, int h) : Fl_Multi_Browser(x, y, w, h) {
	for(const std::pair<int, std::string>& et : KnownEntities::All()) {
		this->add(et.second.c_str());
	}
}

void EntityList::load_list(const std::vector<std::string>& entities) {
	clear();
	for(const std::string& et : entities) {
		add(et.c_str());	
	}

	if(entities.size() > 0) {
		value(1);
	}
}

//computes the minimum required y visibility
int EntityList::selected_y() {
	if(value() == 0)
		return 0;
	else
		return (value()-1)*incr_height();
}

void EntityList::shift_selection(int dy) {
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

int EntityList::handle(int evt) {
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

EntitySearch::EntitySearch(int x, int y, int w, int h) : Fl_Input(x, y, w, h, "Search:") {}

int EntitySearch::handle(int evt) {
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
			EntityList* elist = static_cast<EntityPicker*>(parent())->entity_list;

			if(sval == "") {
				std::vector<std::string> all;
				for(auto&& et : KnownEntities::All()) {
					all.push_back(et.second);
				}

				elist->load_list(all);
			}
			else {
				elist->load_list(KnownEntities::Search(value()));
			}

			last_query = sval;
		}
	}

	return ret;
}

//////////

void PickButton::do_pick() {
	EntityPicker* ep = static_cast<EntityPicker*>(parent());
	EntityList* list = ep->entity_list;
	const char* text = list->text(list->value());
	if(!text) {
		MessageBox(NULL, "No entity type is selected for picking.", "Entity Picker", MB_OK);
		return;
	}

	int id = KnownEntities::GetID(std::string(text));
	if(!id) {
		MessageBox(NULL, "Failed to find ID for the selected entity type, try re-selecting the entity.", "Entity Picker", MB_OK);
		return;
	}

	ep->picker->entity_select(id);
	ep->hide();
	ep->editor->hint_bar->set_entity(id, nullptr);
	ep->editor->parent()->redraw();
	ep->editor->parent()->take_focus();
}

PickButton::PickButton(int x, int y, int w, int h) : Fl_Button(x, y, w, h, "Pick Entity") {
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

EntityPicker::EntityPicker(EditorWidget* editor) :
	Fl_Double_Window(303, 258, "Entity Picker"),
	editor(editor),
	picker(&editor->get_picker())
{
	this->begin();
	{ entity_list = new EntityList(10, 10, 285, 175);
	} // Fl_Browser* o
	{ search_input = new EntitySearch(65, 191, 230, 27);
	} // Fl_Input* o
	{ pick_button = new PickButton(11, 225, 159, 25);
	} // Fl_Button* o
	{ cancel_button = new CancelButton(175, 225, 119, 25);
	} // Fl_Button* o
	this->end();
}

int EntityPicker::handle_input_event(int evt) {
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
			entity_list->shift_selection(ctrl_down ? -2 : -1);
			return 1;

		case 65364: //(ctrl+) down: move selection down
			entity_list->shift_selection(ctrl_down ? 2 : 1);
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

int EntityPicker::handle(int evt) {
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
			if(cursor_event_bounding(entity_list)) {
				entity_list->handle(evt);
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