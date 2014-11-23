#include "tile_editing.h"
#include "tile_editor_widget.h"

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Scrollbar.H>

#include <map>
#include <unordered_map>
#include <boost/assign.hpp> 

//OPT add simulation widget to the overview page, should simulate a chosen area's chunks
static Fl_Window* window = nullptr;
static std::shared_ptr<TilePatch> tp;	
static std::function<void(bool)> display_cb;

typedef std::vector<std::pair<std::string, std::string>> area_map;
static area_map area_lookup = boost::assign::map_list_of
			("Tutorial", "LevelGen_Tutorial")		
			("Mines (1-1 to 1-4)", "LevelGen_Mines")
			("Jungle (2-1 to 2-4)", "LevelGen_JungleGeneral")
			("Ice Caves (3-1 to 3-4)", "LevelGen_IceCavesGeneral")
			("Temple (4-1 to 4-3)", "LevelGen_Temple")
			("Olmec (4-4)", "LevelGen_Olmec")
			("Hell (5-1 to 5-3)", "LevelGen_Hell")
			("Worm", "LevelGen_Worm")
			("Haunted Castle", "LevelGen_JungleHauntedMansion")
			("Black Market", "LevelGen_JungleBlackMarket")
			("Yeti Level", "LevelGen_IceCavesYeti")
			("The Mothership", "LevelGen_IceCavesSpaceship");

template <typename KeyType, typename ValueType>
static const std::pair<KeyType, ValueType>& mget(const std::vector<std::pair<KeyType, ValueType>>& m, const KeyType& key) {
	for(const std::pair<KeyType, ValueType>& p : m) {
		if(p.first == key)
			return p;
	}
	
	throw std::runtime_error("Key not found.");
}


struct AreaControls {
	std::string narea;
	Fl_Button* edit_btn;

	AreaControls(const std::string& narea, Fl_Button* edit_btn, Fl_Box* data) : 
		narea(narea),
		edit_btn(edit_btn)
	{}

	//updates widgets with correct data for this area
	void update() {
		const std::string& rarea = mget(area_lookup, narea).second;

		std::vector<Chunk*> chunks = tp->query_chunks(rarea);
		
		//no chunks, nothing to edit
		if(chunks.empty()) {
			edit_btn->deactivate();
			return;
		}

		edit_btn->activate();
	}
};

static std::map<std::string, AreaControls*> controls;

namespace TileEditing {
	void DisplayStateCallback(std::function<void(bool)> cb) {
		display_cb = cb;
	}

	static std::string current_area_editor;
	static std::map<std::string, EditorWidget*> editors;
	static void SetCurrentEditor(const std::string& area) {
		if(area == current_area_editor)
			return;
		
		if(!current_area_editor.empty()) {
			controls[current_area_editor]->edit_btn->activate();
		}

		controls[area]->edit_btn->deactivate();

		auto ed = editors[area];
		
		if(ed) {
			if(current_area_editor != "") {
				window->remove(editors[current_area_editor]);
			}

			EditorWidget* ew = editors[current_area_editor = area];
			window->add(ew);
			window->add(ew->sidebar_scrollbar);

			window->redraw();
		}
		else {
			throw std::runtime_error("No such area editor");
		}
	}

	struct DoneButton : public Fl_Button {
		DoneButton(int x, int y, int w, int h) : Fl_Button(x,y,w,h, "Close Editor") {}
		int DoneButton::handle(int evt) override {
			if(evt == 2) {
				//TODO
			}
			return Fl_Button::handle(evt);
		}
	};

	struct RevertButton : public Fl_Button {
		RevertButton(int x, int y, int w, int h) : Fl_Button(x,y,w,h, "Revert Changes") {}
		int RevertButton::handle(int evt) override {
			if(evt == 2) {
				//TODO
			}
			return Fl_Button::handle(evt);
		}
	};

	struct SaveButton : public Fl_Button {
		SaveButton(int x, int y, int w, int h) : Fl_Button(x,y,w,h, "Save Changes") {}
		int SaveButton::handle(int evt) override {
			if(evt == 2) {
				//TODO
			}
			return Fl_Button::handle(evt);
		}
	};

	
	struct AreaButton : public Fl_Button {
		std::string area;

		AreaButton(int x, int y, int w, int h, const std::string& narea) : 
			Fl_Button(x,y,w,h),
			area(narea)
		{
			this->copy_label(narea.c_str());
		}

		virtual int handle(int evt) override {
			if(evt == 2) {
				SetCurrentEditor(area);
			}
			return Fl_Button::handle(evt);
		}
	};

	//lays out the UI widgets and stores them to an AreaControls object.
	static void create_controls(int x, int y, const std::string& narea) {
		const std::string& rarea = mget(area_lookup, narea).second;
		Fl_Button* btn = new AreaButton(x, y, 150, 25, narea);
		Fl_Box* data = new Fl_Box(x+160, y, 1, 25, "");
		data->align(FL_ALIGN_RIGHT);
		
		if(!tp->valid()) {
			btn->deactivate();
			return;
		}

		controls[narea] = new AreaControls(narea, btn, data);
		controls[narea]->update();
	}

	static void construct_window() {
		Fl_Window* cons = new Fl_Double_Window(730, 430, "Level Editor Overview");
		window = cons;

		cons->begin();

		int y = 5;
		for(auto&& area : area_lookup) {
			create_controls(5, y, area.first);
			y += 27;
		}

		y += 5;

		new SaveButton(5, y, 80, 25);
		new RevertButton(90, y, 80, 25);
		y += 30;
		new DoneButton(5, y, 160, 25);

		cons->end();

		for(auto&& area : area_lookup) {
			EditorScrollbar* es = new EditorScrollbar(165, 5, 15, cons->h() - 10);
			EditorWidget* ew = new EditorWidget(190, 5, 595, cons->h() - 10, es, tp->query_chunks(area.second));
			es->set_parent_editor(ew);
			editors[area.first] = ew;
		}

		SetCurrentEditor(area_lookup.begin()->first);
	}

	bool Initialize(std::shared_ptr<Spelunky> spel, std::shared_ptr<DerandomizePatch> dp, std::shared_ptr<GameHooks> gh) {
		//DEBUG (After UI is implemented this will only initialize on use)
		tp = std::make_shared<TilePatch>(spel);
		if(!tp->valid()) {
			return false;
		}

		construct_window();
		window->callback([](Fl_Widget* widget) {
			display_cb(false);
			static_cast<Fl_Window*>(widget)->hide();
		});

		return true;
	}

	void ShowUI() {
		if(window) {
			window->show();
			display_cb(true);
		}
	}

	void HideUI() {
		if(window) {
			window->hide();
			display_cb(false);
		}
	}

	bool Visible() {
		return window->visible() != 0;
	}

	bool Valid() {
		return tp && tp->valid();
	}

	std::shared_ptr<::Patch> GetPatch() {
		return tp;
	}
}