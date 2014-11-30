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


typedef struct area_grouping {
	std::vector<std::string> areas;

	area_grouping(std::string a, std::string b="", std::string c="", std::string d="", std::string e="", std::string f="") {
		areas.push_back(a);
		if(!b.empty())
			areas.push_back(b);
		if(!c.empty())
			areas.push_back(c);
		if(!d.empty())
			areas.push_back(d);
		if(!e.empty())
			areas.push_back(e);
		if(!f.empty())
			areas.push_back(f);
	}

	area_grouping() {}
} area_grouping;

typedef std::vector<std::pair<std::string, std::string>> area_map;
typedef std::vector<std::pair<std::string, area_grouping>> grouping_map;

static area_map area_lookup = boost::assign::map_list_of
			("Tutorial", "Tutorial")		
			("1-1", "Mines-1")
			("1-2", "Mines-2")
			("1-3", "Mines-3")
			("1-4", "Mines-4")
			("2-1", "Jungle-5")
			("2-2", "Jungle-6")
			("2-3", "Jungle-7")
			("2-4", "Jungle-8")
			("3-1", "IceCaves-9")
			("3-2", "IceCaves-10")
			("3-3", "IceCaves-11")
			("3-4", "IceCaves-12")
			("4-1", "Temple-13")
			("4-2", "Temple-14")
			("4-3", "Temple-15")
			("Olmec (4-4)", "TempleOlmec")
			("5-1", "Hell-17")
			("5-2", "Hell-18")
			("5-3", "Hell-19")
			("Worm", "Worm")
			("Black Market", "JungleBlackMarket")
			("Haunted Castle", "JungleHauntedCastle")
			("Yeti Level", "IceCavesYeti")
			("The Mothership", "IceCavesSpaceship");

static grouping_map grouping = boost::assign::map_list_of
	("Tutorial", area_grouping("Tutorial"))
	("Mines", area_grouping("1-1", "1-2", "1-3", "1-4"))
	("Jungle", area_grouping("2-1", "2-2", "2-3", "2-4"))
	("Ice Caves", area_grouping("3-1", "3-2", "3-3", "3-4"))
	("Temple", area_grouping("4-1", "4-2", "4-3"))
	("Olmec (4-4)", area_grouping("Olmec (4-4)"))
	("Hell", area_grouping("5-1", "5-2", "5-3"))
	("Black Market", area_grouping("Black Market"))
	("Haunted Castle", area_grouping("Haunted Castle"))
	("Worm", area_grouping("Worm"))
	("Yeti Level", area_grouping("Yeti Level"))
	("The Mothership", area_grouping("The Mothership"));

static std::string area_group(const std::string& area) {
	for(auto&& p : grouping) {
		if(std::find(p.second.areas.begin(), p.second.areas.end(), area) != p.second.areas.end())
			return p.first;
	}
	return "";
}

template <typename KeyType, typename ValueType>
static const std::pair<KeyType, ValueType>& mget(const std::vector<std::pair<KeyType, ValueType>>& m, const KeyType& key) {
	for(const std::pair<KeyType, ValueType>& p : m) {
		if(p.first == key)
			return p;
	}
	
	throw std::runtime_error("Key not found.");
}


struct AreaControls {
	std::string group;
	std::map<std::string, Fl_Button*> btns;

	AreaControls(const std::string& group, const std::map<std::string, Fl_Button*>& btns, Fl_Box* data) : 
		group(group),
		btns(btns)
	{}

	//updates widgets with correct data for this area
	void update() {
		for(auto&& btn : btns) {
			Fl_Button* edit_btn = btn.second;

			const std::string& rarea = mget(area_lookup, btn.first).second;
			std::vector<Chunk*> chunks = tp->query_chunks(rarea);
		
			if(!tp->valid() || chunks.empty()) {
				edit_btn->deactivate();
				continue;
			}

			edit_btn->activate();
		}
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
			controls[area_group(current_area_editor)]->btns[current_area_editor]->activate();
		}

		controls[area_group(area)]->btns[area]->deactivate();

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
	static void create_controls(int x, int y, const std::string& group) {
		const std::vector<std::string>& areas = mget(grouping, group).second.areas;
		
		int p = 0;
		auto make = [&](const std::string& narea) -> Fl_Button* {
			if(areas.size() == 4 || areas.size() == 3) {
				switch(p++) {
				case 0:
					return new AreaButton(x, y, 75, 18, narea);
				case 1:
					return new AreaButton(x+75, y, 75, 18,narea);
				case 2:
					return new AreaButton(x, y+20, 75, 18, narea);
				case 3:
					return new AreaButton(x+75, y+20, 75, 18, narea);
				default:
					return new AreaButton(x, y, 75, 18, narea); //invalid formatting
				}
			}
			else {
				p++;
				return new AreaButton(x, y, 150, 20, narea);
			}
		};

		std::map<std::string, Fl_Button*> buttons;
		//TODO... position the buttons according to their group / group size
		for(const std::string& narea : areas) {
			buttons[narea] = make(narea);
		}
		
		Fl_Box* data = new Fl_Box(x+160, y, 1, 25, "");
		data->align(FL_ALIGN_RIGHT);
		
		controls[group] = new AreaControls(group, buttons, data);
		controls[group]->update();
	}

	static void construct_window() {
		Fl_Window* cons = new Fl_Double_Window(730, 480, "Level Editor Overview");
		window = cons;

		cons->begin();

		int y = 5;
		for(auto it = grouping.begin(); it != grouping.end(); ++it) {
			create_controls(5, y, it->first);
			if(it->second.areas.size() == 4 || it->second.areas.size() == 3)
				y += 43;
			else {
				y += 22;
				
				if(it + 1 != grouping.end() && ((it+1)->second.areas.size() == 4 || (it+1)->second.areas.size() == 3))
					y += 2;
			}
		}

		y -= 20;

		new SaveButton(5, y += 30, 150, 25);
		new LoadButton(5, y += 30, 150, 25);
		new RevertButton(5, y += 30, 150, 25);

		cons->end();

		std::string valid_editor;
		for(auto&& area : area_lookup) {
			EditorScrollbar* es = new EditorScrollbar(710, 5, 15, 420);
			std::vector<Chunk*> chunks = tp->query_chunks(area.second);
			if(!chunks.empty()) {
				if(valid_editor.empty()) {
					valid_editor = area.first;
				}

				EditorWidget* ew = new EditorWidget(tp, 165, 5, 545, 420, es, chunks);
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