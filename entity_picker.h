#pragma once

#include "tile_editor_widget.h"

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Input.h>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include "tile_picker.h"
#include "known_entities.h"

#include <iostream>
#include <iomanip>

class EntityList : public Fl_Multi_Browser {
private:
	int selected_y();

public:
	EntityList(int x, int y, int w, int h);

	void load_list(const std::vector<std::string>& entities);
	void shift_selection(int dy);

	virtual int handle(int evt) override;
};

class EntitySearch : public Fl_Input {
private:
	std::string last_query;

public:
	EntitySearch(int x, int y, int w, int h);

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

class SpecCheckButton : public Fl_Check_Button {
public:
	SpecCheckButton(int x, int y, int w, int h, const char* L);

	virtual int handle(int evt) override;
};

class EntityPicker : public Fl_Double_Window {
private:
	EditorWidget* editor;
	TilePicker* picker;
	EntityList* entity_list;
	EntitySearch* search_input;
	PickButton* pick_button;
	CancelButton* cancel_button;

	SpecCheckButton* bg_w;

public:
	EntityPicker(EditorWidget* editor);
	
	int handle_input_event(int evt);
	virtual int handle(int evt) override;

	friend class TilePicker;
	friend class EntityList;
	friend class EntitySearch;
	friend class PickButton;
	friend class CancelButton;
};