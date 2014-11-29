#include "tile_editing.h"
#include "tile_util.h"
#include "tile_editor_widget.h"

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Scrollbar.H>

#include <map>
#include <unordered_map>
#include <fstream>

#include <boost/assign.hpp> 
#include <pugixml.hpp>

#define WINDOW_BASE_TITLE "Tile Editor"

//OPT add simulation widget to the overview page, should simulate a chosen area's chunks
static Fl_Window* window = nullptr;
static std::shared_ptr<StaticChunkPatch> tp;	
static std::function<void(bool)> display_cb;

typedef std::vector<std::pair<std::string, std::string>> area_map;
/*static area_map area_lookup = boost::assign::map_list_of
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
			("The Mothership", "LevelGen_IceCavesSpaceship");*/
static area_map area_lookup = boost::assign::map_list_of
			("Tutorial", "Tutorial")		
			("Mines (1-1 to 1-4)", "Mines")
			("Jungle (2-1 to 2-4)", "Jungle")
			("Ice Caves (3-1 to 3-4)", "IceCaves")
			("Temple (4-1 to 4-3)", "Temple")
			("Olmec (4-4)", "TempleOlmec")
			("Hell (5-1 to 5-3)", "Hell")
			("Worm", "Worm")
			("Haunted Castle", "JungleHauntedCastle")
			("Black Market", "JungleBlackMarket")
			("Yeti Level", "IceCavesYeti")
			("The Mothership", "IceCavesSpaceship");

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
		
		if(!tp->valid() || chunks.empty()) {
			edit_btn->deactivate();
			return;
		}

		edit_btn->activate();
	}
};

static std::map<std::string, AreaControls*> controls;


namespace TileEditing {
	static std::string current_area_editor;
	static std::map<std::string, EditorWidget*> editors;
	
	namespace IO 
	{
		static std::string current_file;	
		static bool unsaved_changes = false;
		
		static void status_handler(unsigned state) {
			if(state == STATE_CHUNK_WRITE || state == STATE_CHUNK_PASTE) {
				window->copy_label((std::string(WINDOW_BASE_TITLE " - ") + current_file + "**").c_str());
				unsaved_changes = true;
			}
			else if(state == STATE_CHUNK_APPLY) {
				if(unsaved_changes)
					window->copy_label((std::string(WINDOW_BASE_TITLE " - ") + current_file + "*").c_str());
				else
					window->copy_label((std::string(WINDOW_BASE_TITLE " - ") + current_file).c_str());
			}
		}

		static void SetActiveFile(const std::string& file) {
			current_file = file;

			if(file == "")
				window->label(WINDOW_BASE_TITLE);
			else
				window->copy_label((std::string(WINDOW_BASE_TITLE " - ") + TileUtil::GetBaseFilename(current_file)).c_str());
			
			tp->apply_chunks(); //apply chunks upon setting active file to guarantee newly active file is not unapplied
		}

		static void LoadFile(const std::string& file) {
			std::ifstream fst(file, std::ios::in);
			if(!fst.is_open()) {
				//create file if not exists
				std::ofstream ofs(file, std::ios::out);
				if(!ofs.is_open()) {
					throw std::runtime_error("Failed to create file.");
				}
				else {
					SetActiveFile(file);
					ofs.close();
				}
			}
			else {
				fst.close();
				pugi::xml_document xmld;
				if(!xmld.load_file(file.c_str())) {
					throw std::runtime_error("XML Parser failed to load file.");
				}			

				std::vector<SingleChunk*> scs = tp->root_chunks();
				pugi::xml_node chunks = xmld.child("chunks");
				for(pugi::xml_node cnk : chunks) {
					if(std::distance(cnk.children().begin(), cnk.children().end()) != 1)
						throw std::runtime_error("Invalid format.");

					pugi::xml_node data = *(cnk.children().begin());
					if(data.type() != pugi::xml_node_type::node_pcdata)  {
						throw std::runtime_error("Invalid node format, expected node_pcdata.");
					}

					std::string str(cnk.name());
					std::string val(data.value());
					for(SingleChunk* sc : scs) {
						if(sc->get_name() == str) {
							if(sc->get_data().size() == val.size()) {
								sc->set_data(val);
							}
							break;
						}
					}
				}

				SetActiveFile(current_file);
			}
		}
		
		static void _singlechunks(Chunk* c, std::function<void(Chunk*)> cb) {
			if(c->type() == ChunkType::Group) {
				for(Chunk* ck : static_cast<GroupChunk*>(c)->get_chunks()) {
					_singlechunks(ck, cb);
				}
			}
			else {
				cb(c);
			}
		}

		static void EncodeToFile() {
			pugi::xml_document xmld;
			pugi::xml_node node = xmld.append_child("chunks");
			if(!node) { 
				throw std::runtime_error("Failed to create chunks node.");	
			}

			for(SingleChunk* c : tp->root_chunks()) {
				pugi::xml_node cnkn = node.append_child(c->get_name().c_str());
				pugi::xml_node data = cnkn.append_child(pugi::xml_node_type::node_pcdata);
				data.set_value(static_cast<SingleChunk*>(c)->get_data().c_str());
			}

			if(!xmld.save_file(current_file.c_str())) {
				throw std::runtime_error("Failed to write document.");
			}

			SetActiveFile(current_file);

			unsaved_changes = false;
		}

		static const std::string& CurrentFile() {
			return current_file;
		}
	}

	
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

			ew->take_focus();
			window->redraw();
		}
		else {
			throw std::runtime_error("No such area editor");
		}
	}

	void DisplayStateCallback(std::function<void(bool)> cb) {
		display_cb = cb;
	}

	void revert_chunks() {
		//TODO
	}

	BUTTON_CLASS(RevertButton, "New File");
	int RevertButton::handle(int evt) {
		if(evt == 2) {
			revert_chunks();
			IO::SetActiveFile("");
		}
		else if(evt == FL_FOCUS)
			return 0;
		return Fl_Button::handle(evt);
	}

	BUTTON_CLASS(SaveButton, "Save to File");
	int SaveButton::handle(int evt) {
		if(evt == 2) {
			try {
				if(IO::current_file != "") {
					IO::EncodeToFile();	
				}
				else {
					try {
						std::string file = TileUtil::QueryTileFile(true);
						try {
							IO::SetActiveFile(file);
							IO::EncodeToFile();
						}
						catch(std::exception& e) {
							MessageBox(NULL, (std::string("Error saving file: ") + e.what()).c_str(), "Error", MB_OK);
						}
					}
					catch(std::exception&) {}
				}
			}
			catch(std::exception&) {}
		}
		else if(evt == FL_FOCUS)
			return 0;
		return Fl_Button::handle(evt);
	}
	
	BUTTON_CLASS(LoadButton, "Load File");
	int LoadButton::handle(int evt) {
		if(evt == 2) {
			try {
				std::string file = TileUtil::QueryTileFile(false);
				try {
					IO::LoadFile(file);
					::window->redraw();
				}
				catch(std::exception& e) {
					MessageBox(NULL, (std::string("Error loading file: ") + e.what()).c_str(), "Error", MB_OK);
				}
			}
			catch(std::exception&) {}
		}
		else if(evt == FL_FOCUS)
			return 0;

		return Fl_Button::handle(evt);	
	}
	
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
			else if(evt == FL_FOCUS)
				return 0;
			return Fl_Button::handle(evt);
		}
	};

	//lays out the UI widgets and stores them to an AreaControls object.
	static void create_controls(int x, int y, const std::string& narea) {
		const std::string& rarea = mget(area_lookup, narea).second;

		Fl_Button* btn = new AreaButton(x, y, 150, 25, narea);
		Fl_Box* data = new Fl_Box(x+160, y, 1, 25, "");
		data->align(FL_ALIGN_RIGHT);
		
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

		y -= 20;

		new SaveButton(5, y += 30, 150, 25);
		new LoadButton(5, y += 30, 150, 25);
		new RevertButton(5, y += 30, 150, 25);

		cons->end();

		std::string valid_editor;
		for(auto&& area : area_lookup) {
			EditorScrollbar* es = new EditorScrollbar(710, 5, 15, cons->h() - 10);
			std::vector<Chunk*> chunks = tp->query_chunks(area.second);
			if(!chunks.empty()) {
				if(valid_editor.empty()) {
					valid_editor = area.first;
				}

				EditorWidget* ew = new EditorWidget(tp, 165, 5, 545, cons->h() - 10, es, chunks);
				es->set_parent_editor(ew);
				ew->status_callback(IO::status_handler);
				editors[area.first] = ew;
			}
		}

		if(!valid_editor.empty()) {
			SetCurrentEditor(valid_editor);
		}
	}

	bool Initialize(std::shared_ptr<Seeder> seeder, std::shared_ptr<StaticChunkPatch> scp) {
		tp = scp;

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