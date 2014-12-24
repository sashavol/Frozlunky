#include "tile_editing.h"
#include "tile_util.h"
#include "tile_default.h"
#include "tile_editor_widget.h"
#include "tile_editing_menubar.h"
#include "tile_editing_hintbar.h"
#include "level_forcer.h"
#include "resource_editor.h"
#include "resource_editor_gui.h"
#include "level_settings_gui.h"
#include "entity_picker.h"
#include "gui.h"
#include "syllabic.h"

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Input.H>

#include <mutex>
#include <map>
#include <unordered_map>
#include <fstream>
#include <functional>

#include <boost/assign.hpp> 
#include <boost/algorithm/string.hpp>
#include <pugixml.hpp>

#define WINDOW_WIDTH  780
#define WINDOW_HEIGHT 510
#define WINDOW_BASE_TITLE "Level Editor"

#define BUTTON_CLASS(NAME, LABEL) \
class NAME : public Fl_Button { \
public: \
	NAME(int x, int y, int w, int h) : Fl_Button(x,y,w,h,LABEL) {} \
	virtual int handle(int evt) override; \
}

template <typename Numeric_>
static Numeric_ clamp_(Numeric_ s, Numeric_ e, Numeric_ v) {
	return min(e, max(s, v));
}


//OPT add simulation widget to the overview page, should simulate a chosen area's chunks
static Fl_Window* window = nullptr;
static Fl_Group* editor_group = nullptr;
static std::shared_ptr<StaticChunkPatch> tp;	
static std::shared_ptr<LevelForcer> level_forcer;
static std::shared_ptr<LevelRedirect> level_redirect;
static std::shared_ptr<Seeder> seeder;
static std::shared_ptr<DerandomizePatch> dp;
static std::shared_ptr<GameHooks> gh;
static std::shared_ptr<ResourceEditor> resource_editor;
static ResourceEditorWindow* resource_editor_window;
static LevelSettingsWindow* level_settings_window;
static HANDLE worker_thread = 0;
static std::function<void(bool)> display_cb;
static bool initialized = false;

struct NF_CheckButton : public Fl_Check_Button {
public:
	NF_CheckButton(int x, int y, int w, int h, const char* L) : 
		Fl_Check_Button(x,y,w,h,L)
	{}

	virtual int handle(int evt) {
		if(evt == FL_FOCUS)
			return 0;
		else
			return Fl_Check_Button::handle(evt);
	}
};

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

//mapping references:
//	tile_editing.cpp
//	tile_draw.cpp
static area_map area_lookup = boost::assign::map_list_of
			("Default (Read-Only)", "%")
			("Tut-1", "Tutorial-1")
			("Tut-2", "Tutorial-2")
			("Tut-3", "Tutorial-3")
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
			("Yama (5-4)", "Hell-20")
			("Worm", "Worm")
			("Black Market", "JungleBlackMarket")
			("Haunted Castle", "JungleHauntedCastle");
			//("Yeti Level", "IceCavesYeti")
			//("The Mothership", "IceCavesSpaceship");

static grouping_map grouping = boost::assign::map_list_of
	("Default (Read-Only)", area_grouping("Default (Read-Only)"))
	("Tutorial", area_grouping("Tut-1", "Tut-2", "Tut-3"))
	("Mines", area_grouping("1-1", "1-2", "1-3", "1-4"))
	("Jungle", area_grouping("2-1", "2-2", "2-3", "2-4"))
	("Ice Caves", area_grouping("3-1", "3-2", "3-3", "3-4"))
	("Temple", area_grouping("4-1", "4-2", "4-3"))
	("Olmec (4-4)", area_grouping("Olmec (4-4)"))
	("Hell", area_grouping("5-1", "5-2", "5-3"))
	("Yama (5-4)", area_grouping("Yama (5-4)"))
	("Black Market", area_grouping("Black Market"))
	("Haunted Castle", area_grouping("Haunted Castle"))
	("Worm", area_grouping("Worm"));
	//("Yeti Level", area_grouping("Yeti Level"))
	//("The Mothership", area_grouping("The Mothership"));

static void area_order(std::vector<std::string>& areas) {
	for(auto&& p : grouping) {
		for(const std::string& s : p.second.areas) {
			areas.push_back(s);
		}
	}
}

typedef std::function<void()> fnv_;
static std::map<std::string, fnv_> level_force_oper = boost::assign::map_list_of
	("1-1",			fnv_([](){level_forcer->force(1);}))
	("1-2",			fnv_([](){level_forcer->force(2);}))
	("1-3",		    fnv_([](){level_forcer->force(3);}))
	("1-4",		    fnv_([](){level_forcer->force(4);}))
	("2-1",			fnv_([](){level_forcer->force(5);}))
	("2-2",			fnv_([](){level_forcer->force(6);}))
	("2-3",			fnv_([](){level_forcer->force(7);}))
	("2-4",			fnv_([](){level_forcer->force(8);}))
	("3-1",			fnv_([](){level_forcer->force(9);}))
	("3-2",			fnv_([](){level_forcer->force(10);}))
	("3-3",			fnv_([](){level_forcer->force(11);}))
	("3-4",			fnv_([](){level_forcer->force(12);}))
	("4-1",			fnv_([](){level_forcer->force(13);}))
	("4-2",			fnv_([](){level_forcer->force(14);}))
	("4-3",			fnv_([](){level_forcer->force(15);}))
	("Olmec (4-4)", fnv_([](){level_forcer->force(16);}))
	("5-1",			fnv_([](){level_forcer->force(17);}))
	("5-2",			fnv_([](){level_forcer->force(18);}))
	("5-3",			fnv_([](){level_forcer->force(19);}))
	("Yama (5-4)",  fnv_([](){level_forcer->force(20);}))
	("Black Market", fnv_([](){level_forcer->force(6, LF_BLACK_MARKET);}))
	("Haunted Castle", fnv_([](){level_forcer->force(5, LF_HAUNTED_MANSION);}))
	("Worm",		fnv_([](){level_forcer->force(5, LF_WORM);}));
	//("Yeti Level",  std::function<void()>([](){level_forcer->force(9, LF_YETI);}))
	//("The Mothership", std::function<void()>([](){level_forcer->force(12, LF_MOTHERSHIP);}));

static std::string current_game_level() {
	Address game;
	dp->spel->read_mem(dp->game_ptr(), &game, sizeof(Address));

	bool black_market;
	dp->spel->read_mem(game + gh->blackmkt_offset(), &black_market, 1);
	if(black_market)
		return "Black Market";

	bool worm;
	dp->spel->read_mem(game + gh->worm_offset(), &worm, 1);
	if(worm)
		return "Worm";
	
	bool haunted_mansion;
	dp->spel->read_mem(game + gh->haunted_mansion_offset(), &haunted_mansion, 1);
	if(haunted_mansion)
		return "Haunted Castle";

	bool mothership;
	dp->spel->read_mem(game + gh->mothership_offset(), &mothership, 1);
	if(mothership)
		return "The Mothership";

	int level = dp->current_level();
	switch(level) {
	case 16:
		return "Olmec (4-4)";
	case 20:
		return "Yama (5-4)";
	default:
		return std::to_string(1 + (level-1) / 4) + "-" + std::to_string(((level - 1) % 4) + 1);
	}
}

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
			
			//% -> default read-only
			if(rarea != "%") {
				std::vector<Chunk*> chunks = tp->query_chunks(rarea);
		
				if(!tp->valid() || chunks.empty()) {
					edit_btn->deactivate();
					continue;
				}
			}

			edit_btn->activate();
		}
	}
};


static std::map<std::string, AreaControls*> controls;

namespace TileEditing {
	static std::string prior_nondefault_editor;
	static std::string current_area_editor;
	static std::map<std::string, EditorWidget*> editors;
	static Fl_Check_Button* flcb_force;

	static void SetCurrentEditor(const std::string& area);
	static void ForceCurrentLevel(bool r);

	namespace IO 
	{
		static std::string current_file;	
		static bool unsaved_changes = false;
		
		static void SetActiveFile(const std::string& file) {
			current_file = file;

			if(file == "")
				window->label(WINDOW_BASE_TITLE);
			else
				window->copy_label((std::string(WINDOW_BASE_TITLE " - ") + TileUtil::GetBaseFilename(current_file)).c_str());
			
			tp->apply_chunks(); //apply chunks upon setting active file to guarantee newly active file is not unapplied
		}

		static void InitializeEmptyEntities() {
			for(auto&& e : editors) {
				std::shared_ptr<EntitySpawnBuilder> esb = e.second->get_entity_builder();
				if(esb) {
					e.second->get_picker().get_recent_entities().clear();
					esb->clear();
				}
			}
		}

		static void NewFile() {
			TileDefault::SetToDefault(tp->get_chunks());

			InitializeEmptyEntities();

			resource_editor->reset();
			resource_editor_window->update();

			level_redirect->reset();
			level_settings_window->update();

			IO::SetActiveFile("");
			unsaved_changes = false;
		}

		static std::string SafeXMLName(std::string level) {
			using namespace boost::algorithm;
			
			replace_all(level, " ", "-");
			replace_all(level, "(", "-");
			replace_all(level, ")", "-");

			return std::string("s")+level;
		}

		static std::string AreaName(const std::string& safe) {
			for(auto&& level : area_lookup) {
				if(SafeXMLName(level.first) == safe) {
					return level.first;
				}
			}
			return "";
		}

		static void EncodeToFile() {
			pugi::xml_document xmld;
	
			pugi::xml_node settings = xmld.append_child("settings");
			{
				pugi::xml_node redirect = settings.append_child("redirect");
				redirect.append_child("start").append_attribute("to").set_value(level_redirect->level_start);
				redirect.append_child("olmec").append_attribute("to").set_value(level_redirect->level_olmec);
				redirect.append_child("yama").append_attribute("to").set_value(level_redirect->level_yama);

				settings.append_child("checkpoints").append_attribute("active").set_value(level_redirect->checkpoint_mode);
			}

			pugi::xml_node resources = xmld.append_child("resources");
			for(auto&& res : *resource_editor) {
				if(mget(area_lookup, res.first).second == "%")
					continue;

				pugi::xml_node level = resources.append_child(SafeXMLName(res.first).c_str());

				auto& r = res.second;
				level.append_attribute("bombs").set_value(r.bombs);
				level.append_attribute("ropes").set_value(r.ropes);
				level.append_attribute("health").set_value(r.health);
			}

			pugi::xml_node entities = xmld.append_child("entities");
			for(auto&& e : editors) {
				if(mget(area_lookup, e.first).second == "%")
					continue;

				std::shared_ptr<EntitySpawnBuilder> esb = e.second->get_entity_builder();
				if(std::distance(esb->begin(), esb->end()) == 0)
					continue;
				
				pugi::xml_node level = entities.append_child(SafeXMLName(e.first).c_str());

				if(esb) {
					for(auto&& et : *esb) {
						pugi::xml_node entity = level.append_child("entity");
						entity.append_attribute("id").set_value(et.second.entity);
						entity.append_attribute("x").set_value(et.second.x);
						entity.append_attribute("y").set_value(et.second.y);
					}
				}
			}

			pugi::xml_node editorsn = xmld.append_child("editors");
			for(auto&& e : editors) {
				if(e.second->get_entity_builder()) {
					std::vector<int>& recent_entities = e.second->get_picker().get_recent_entities();
					if(recent_entities.empty())
						continue;
					
					pugi::xml_node level = editorsn.append_child(SafeXMLName(e.first).c_str());
					pugi::xml_node picker = level.append_child("picker");
					for(int entity : recent_entities) {
						picker.append_child("entity").append_attribute("id").set_value(entity);
					}
				}
			}

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

		static void LoadFile(const std::string& file) {
			std::ifstream fst(file, std::ios::in);
			if(!fst.is_open()) {
				//create file if not exists
				std::ofstream ofs(file, std::ios::out);
				if(!ofs.is_open()) {
					throw std::runtime_error("Failed to create file.");
				}
				else {
					//clear undos
					for(auto&& ep : editors) {
						ep.second->clear_state();
					}

					SetActiveFile(file);
					ofs.close();
				}

				unsaved_changes = false;
			}
			else {
				fst.close();
				pugi::xml_document xmld;
				if(!xmld.load_file(file.c_str())) {
					throw std::runtime_error("XML Parser failed to load file.");
				}

				//clear undos
				for(auto&& ep : editors) {
					ep.second->clear_state();
				}

				request_soft_seed_lock();

				level_redirect->reset();
				pugi::xml_node settings = xmld.child("settings");
				if(!settings.empty()) {
					pugi::xml_node redirect = settings.child("redirect");
					if(!redirect.empty()) {
						auto level_proc = [=](const char* elem, std::atomic<int>& target) {
							pugi::xml_node n = redirect.child(elem);
							if(!n.attribute("to").empty()) {
								target = clamp_(LEVEL_1_1, LEVEL_5_4, n.attribute("to").as_int());
							}
						};
						
						level_proc("start", level_redirect->level_start);
						level_proc("olmec", level_redirect->level_olmec);
						level_proc("yama", level_redirect->level_yama);
					}

					pugi::xml_node checkpoints = settings.child("checkpoints");
					if(!checkpoints.empty()) {
						pugi::xml_attribute active = checkpoints.attribute("active");
						if(!active.empty()) {
							level_redirect->checkpoint_mode = active.as_bool();
						}
					}
				}
				level_settings_window->update();

				resource_editor->reset();
				pugi::xml_node resources = xmld.child("resources");
				if(!resources.empty()) {
					for(pugi::xml_node level : resources.children()) {
						std::string area = AreaName(level.name());
						
						if(area != "") {
							ResourceEditor::Resources& res = resource_editor->res(area);
							
							pugi::xml_attribute bombsa = level.attribute("bombs"),
												ropesa = level.attribute("ropes"),
												healtha = level.attribute("health");

							if(bombsa.empty() || ropesa.empty() || healtha.empty()) {
								throw std::runtime_error("Encountered invalid resource specifier.");
							}

							res.bombs = bombsa.as_int();
							res.ropes = ropesa.as_int();
							res.health = healtha.as_int();
						}
					}
				}
				resource_editor_window->update();


				std::vector<SingleChunk*> scs = tp->root_chunks();
				pugi::xml_node chunks = xmld.child("chunks");
				if(chunks.empty()) {
					throw std::runtime_error("Invalid format.");
				}

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


				InitializeEmptyEntities();
				pugi::xml_node entities = xmld.child("entities");
				if(!entities.empty()) {
					for(pugi::xml_node level : entities.children()) {
						std::string area = AreaName(level.name());

						if(area != "") {
							std::shared_ptr<EntitySpawnBuilder> esb = editors[area]->get_entity_builder();

							if(esb) {
								for(pugi::xml_node et : level.children()) {
									if(std::string(et.name()) != "entity")
										continue;

									pugi::xml_attribute xa = et.attribute("x"),
														ya = et.attribute("y"),
														ida = et.attribute("id");

									if(xa.empty() || ya.empty() || ida.empty()) {
										throw std::runtime_error("Encountered invalid entity.");
									}

									esb->add(xa.as_float(), ya.as_float(), ida.as_int());
								}
							}
						}
					}
				}

				pugi::xml_node editorsn = xmld.child("editors");
				if(!editorsn.empty()) {
					for(pugi::xml_node level : editorsn.children()) {
						std::string area = AreaName(level.name());
						if(area == "") {
							continue;
						}

						pugi::xml_node pickern = level.child("picker");
						if(!pickern.empty()) {
							std::vector<int>& picker_entities = editors[area]->get_picker().get_recent_entities();
							for(pugi::xml_node entity : pickern.children()) {
								if(std::string(entity.name()) != "entity") {
									continue;
								}

								pugi::xml_attribute ida = entity.attribute("id");
								if(ida.empty()) {
									throw std::runtime_error("Encountered invalid editor picker");
								}

								int id = ida.as_int();
								if(id != 0) {
									picker_entities.push_back(id);
								}
							}
						}
					}
				}

				SetActiveFile(file);
				unsaved_changes = false;
			}
		}

		static void SaveAs() {
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

		static void Save() {
			try {
				if(!current_file.empty()) {
					IO::EncodeToFile();
					window->copy_label((std::string(WINDOW_BASE_TITLE " - ") + TileUtil::GetBaseFilename(current_file)).c_str());
				}
				else {
					IO::SaveAs();
				}
			}
			catch(std::exception& e) {
				MessageBox(NULL, (std::string("Error saving file: ") + e.what()).c_str(), "Error", MB_OK);
			}
		}

		static void resize_window(double mult) {
			window->resizable(editor_group);
			window->size(int(WINDOW_WIDTH*mult), int(WINDOW_HEIGHT*mult));
			window->size_range(int(WINDOW_WIDTH*mult), int(WINDOW_HEIGHT*mult), int(WINDOW_WIDTH*mult), int(WINDOW_HEIGHT*mult));
		}

		static void status_handler(unsigned state) {
			if(state == STATE_CHUNK_WRITE || state == STATE_CHUNK_PASTE || state == STATE_RESERVED1) {
				window->copy_label((std::string(WINDOW_BASE_TITLE " - ") + TileUtil::GetBaseFilename(current_file) + "* (Unsaved Changes)").c_str());
				
				//OPT: tp->apply_chunks() performs a lot of memory-intensive operations
				//		However, it seems to operate within a reasonable speed on most systems, so
				//		we'll keep automatic chunk apply for now.
				tp->apply_chunks();
				unsaved_changes = true;
			}
			else if(state == STATE_REQ_SAVE) {
				IO::Save();
			}
			else if(state == STATE_REQ_SAVE_AS) {
				IO::SaveAs();
			}
			else if(state == STATE_REQ_OPEN) {
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
			else if(state == STATE_REQ_NEW_FILE) {
				if(unsaved_changes) {
					int res = MessageBox(NULL, "You have unsaved changes, save before making a new file?", "New File", MB_YESNOCANCEL);
					if(res != IDCANCEL) {
						if(res == IDYES) {
							IO::Save();
						}
						
						IO::NewFile();
						::window->redraw();
					}
				}
				else {
					IO::NewFile();
					::window->redraw();
				}
			}
			else if(state == STATE_REQ_TAB || state == STATE_REQ_TAB_REVERSE) {
				std::vector<std::string> areas;
				area_order(areas);

				auto p = std::find(areas.begin(), areas.end(), current_area_editor);

				if(p != areas.end()) {
					if(state == STATE_REQ_TAB) {
						if(p+1 == areas.end())
							return;

						SetCurrentEditor(*(p+1));
					}
					else {
						if(p == areas.begin())
							return;
						
						SetCurrentEditor(*(p-1));
					}
				}
			}
			else if(state == STATE_REQ_RESOURCE_EDITOR) {
				if(!resource_editor_window->visible()) {
					resource_editor_window->show();
				}
			}
			else if(state == STATE_REQ_LEVEL_SETTINGS) {
				if(!level_settings_window->visible()) {
					level_settings_window->show();
				}
			}
			else if(state == STATE_REQ_ENTITY_PICKER) {
				if(!current_area_editor.empty()) {
					EditorWidget* target_area = editors[current_area_editor];
					if(target_area->get_entity_builder()) {
						EntityPicker* picker = new EntityPicker(target_area);
						picker->callback([](Fl_Widget* w) {
							delete static_cast<EntityPicker*>(w);
						});
						picker->show();
					}
				}
			}
			else if(state == STATE_REQ_DEFAULT_SWAP) {
				if(mget(area_lookup, current_area_editor).second == "%") {
					SetCurrentEditor(prior_nondefault_editor);
				}
				else {
					SetCurrentEditor("Default (Read-Only)");
				}
			}
			else if(state == STATE_REQ_TOGGLE_FORCE_LEVEL) {
				if(flcb_force->value()) {
					flcb_force->value(0);
					ForceCurrentLevel(false);
				}
				else {
					flcb_force->value(1);
					ForceCurrentLevel(true);
				}
				::window->redraw();
			}
			else if(state == STATE_REQ_RESIZE_1) {
				resize_window(1);
			}
			else if(state == STATE_REQ_RESIZE_1_5) {
				resize_window(1.3);
			}
			else if(state == STATE_REQ_RESIZE_2) {
				resize_window(1.6);
			}
			else if(state == STATE_REQ_RESIZE_3) {
				resize_window(2.0);
			}
			else if(state == STATE_REQ_RESIZE_4) {
				resize_window(2.5);
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

		static const std::string& CurrentFile() {
			return current_file;
		}
	}

	static void ForceCurrentLevel(bool r) {
		if(r) {
			//if default levels, no operation required, just setting checkbox
			if(mget(area_lookup, current_area_editor).second == "%") {
				level_forcer->set_enabled(true);
				return;
			}

			auto oper = level_force_oper[current_area_editor];
			if(oper) {
				oper();
				level_forcer->set_enabled(true);
			}
			else {
				MessageBox(NULL, ("Level '" + current_area_editor + "' cannot be forced.").c_str(), "Level Force", MB_OK);
				level_forcer->set_enabled(false);
				flcb_force->value(0);
				flcb_force->redraw();
			}
		}
		else
			level_forcer->set_enabled(false);
	}
	
	static void SetCurrentEditor(const std::string& area) {
		if(area == current_area_editor)
			return;
		
		//check if not default
		if(mget(area_lookup, area).second != "%") {
			prior_nondefault_editor = area;
		}

		if(!current_area_editor.empty()) {
			controls[area_group(current_area_editor)]->btns[current_area_editor]->activate();
		}

		controls[area_group(area)]->btns[area]->deactivate();

		auto ed = editors[area];
		
		if(ed) {
			if(current_area_editor != "") {
				EditorWidget* last = editors[current_area_editor];
				EditorWidget* curr = editors[area];
				
				curr->shift_down = last->shift_down;
				curr->alt_down = last->alt_down;
				curr->ctrl_down = last->ctrl_down;
				std::memcpy(curr->mouse_down, last->mouse_down, sizeof(curr->mouse_down));

				last->hide();
				last->sidebar_scrollbar->hide();
				last->hint_bar->hide();
				last->deactivate();
				last->sidebar_scrollbar->hide();
				last->hint_bar->hide();
			}
			current_area_editor = area;

			EditorWidget* ew = editors[area];
			ew->activate();
			if(ew->use_scrollbar) {
				ew->sidebar_scrollbar->activate();
			}
			ew->hint_bar->activate();
			
			//show implicitly invokes FL_UNFOCUS on editor, so we have to store input data on editor switch
			bool shift_down = ew->shift_down, alt_down = ew->alt_down, ctrl_down = ew->ctrl_down, mouse_down[3];
			std::memcpy(mouse_down, ew->mouse_down, sizeof(mouse_down));

			ew->show();
			ew->sidebar_scrollbar->show();
			ew->hint_bar->show();

			std::memcpy(ew->mouse_down, mouse_down, sizeof(mouse_down));
			ew->shift_down = shift_down;
			ew->ctrl_down = ctrl_down;
			ew->alt_down = alt_down;

			ForceCurrentLevel(!!flcb_force->value());

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

	static DWORD __stdcall worker_thread_(void*) {
		while(true) {
			if(tp->is_active()) {
				std::string area = current_game_level();
				
				if(seeder->get_seed() != "~") {
					seeder->seed("~");
				}

				if(!level_forcer->enabled())
					level_redirect->cycle();
				else
					level_redirect->last_checkpoint = int(level_redirect->level_start);

				level_forcer->cycle();
				resource_editor->cycle();

				gh->set_dark_level(false);
			}

			Sleep(2);
		}
	}

	//worker thread: currently used for setting level seed
	static void construct_worker_thread() {
		DWORD tid;
		worker_thread = CreateThread(NULL, 0, worker_thread_, NULL, 0, &tid);
	}


	struct TileEditingWindow : public Fl_Double_Window {
		TileEditingWindow(int x, int y, const char* L) : Fl_Double_Window(x, y, L) {}

		virtual int handle(int evt) override {
			//make sure we don't have lingering key-presses on unfocus
			if(evt == FL_UNFOCUS) {
				for(auto&& e : editors) {
					EditorWidget* editor = e.second;
					editor->alt_down = false;
					editor->shift_down = false;
					editor->ctrl_down = false;
				}
				return 1;
			}
			else if(evt == FL_KEYBOARD) {
				int key = Fl::event_key();
				if(key == 65307) { //esc override to prevent window close
					return 0;
				}
			}

			return Fl_Double_Window::handle(evt);
		}
	};

//menu bar y offset
#define MB_Y_OFFSET 27
	static std::string construct_window() {
		Fl_Window* cons = new TileEditingWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BASE_TITLE);
		window = cons;

		cons->begin();

		//construct menu bar, callback is executed when key event needs execution
		new TileEditingMenuBar(0, 0, WINDOW_WIDTH, 25, [=](TileEditingMenuBar::KeyTrigger trigger) {
			EditorWidget* editor = editors[current_area_editor];
			if(editor) {
				editor->shift_down = trigger.shift;
				editor->alt_down = trigger.alt;
				editor->ctrl_down = trigger.ctrl;
				
				editor->handle_key(trigger.key);
				
				editor->shift_down = false;
				editor->alt_down = false;
				editor->ctrl_down = false;
			}
		});

		int y = 5+MB_Y_OFFSET;
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

		flcb_force = new NF_CheckButton(165, 2+425+MB_Y_OFFSET, 100, 20, "Force level to game");
		flcb_force->value(0);
		
		if(!level_forcer->valid()) {
			flcb_force->deactivate();
		}

		flcb_force->callback([](Fl_Widget* cbox) {
			ForceCurrentLevel(!!static_cast<Fl_Check_Button*>(cbox)->value());
		});


		editor_group = new Fl_Group(165, 5+MB_Y_OFFSET, 615, 420);
		cons->end();

		std::string valid_editor;
		for(auto&& area : area_lookup) {
			EditorScrollbar* es = new EditorScrollbar(690, 5+MB_Y_OFFSET, 15, 420);
			TileEditingHintbar* hint_bar = new TileEditingHintbar(165, window->h()-30+1, es->x() - 165 - 5, 25);

			std::vector<Chunk*> chunks = tp->query_chunks(area.second);

			// % -> default read-only
			if(area.second == "%" || !chunks.empty()) {
				if(valid_editor.empty() || valid_editor.find("Tut") == 0 || valid_editor == "Default (Read-Only)") {
					valid_editor = area.first;
				}

				AreaRenderMode arm = area.second != "%" ? mode_from_name(area.first) : AreaRenderMode::INVALID;
				
				std::shared_ptr<EntitySpawnBuilder> esb;
				if(!chunks.empty()) {
					std::pair<std::shared_ptr<StaticAreaPatch>, int> area_patch = tp->parent(chunks[0]);
					esb = area_patch.first ? area_patch.first->entity_builder(area_patch.second) : nullptr;
				}

				//special handling for worm editor construction
				EditorWidget* ew;
				if(area.first == "Worm") {
					std::vector<Chunk*> edited;
					for(size_t i = 0; i < chunks.size(); i++) {
						if(i % 4 < 2) {
							edited.push_back(chunks[i]);
						}
					}
					ew = new EditorWidget(arm, tp, esb, 165, 5+MB_Y_OFFSET, 615, 420, es, hint_bar, edited, true);
				}
				else if(area.second == "%") { //default read-only
					std::vector<Chunk*> relevant;
					for(SingleChunk* c : tp->tile_patch()->root_chunks()) {
						if(c->get_width() == CHUNK_WIDTH && c->get_height() == CHUNK_HEIGHT)
							relevant.push_back(c);
					}
					ew = new EditorWidget(arm, tp, esb, 165, 5+MB_Y_OFFSET, 615, 420, es, hint_bar, relevant, false, true);
				}
				else {
					ew = new EditorWidget(arm, tp, esb, 165, 5+MB_Y_OFFSET, 615, 420, es, hint_bar, chunks);
				}

				editor_group->add(ew);
				editor_group->add(ew->hint_bar);
				editor_group->add(ew->sidebar_scrollbar);

				ew->hide();
				ew->hint_bar->hide();
				ew->sidebar_scrollbar->hide();
				ew->deactivate();
				ew->hint_bar->deactivate();
				ew->sidebar_scrollbar->hide();

				es->set_parent_editor(ew);
				ew->status_callback(IO::status_handler);
				editors[area.first] = ew;
			}
		}

		return valid_editor;
	}
#undef MB_Y_OFFSET

	//entry point for tile editing UI
	bool Initialize(std::shared_ptr<DerandomizePatch> dp, 
			std::shared_ptr<GameHooks> gh, 
			std::shared_ptr<Seeder> seeder, 
			std::shared_ptr<StaticChunkPatch> scp) 
	{
		tp = scp;
		::seeder = seeder;
		::dp = dp;
		::gh = gh;

		level_forcer = std::make_shared<LevelForcer>(dp, gh);
		level_redirect = std::make_shared<LevelRedirect>(gh);

		{
			std::vector<std::string> areas;
			for(auto&& area : area_lookup) {
				if(area.second != "%") {
					areas.push_back(area.first);
				}
			}
			::resource_editor = std::make_shared<ResourceEditor>(gh, current_game_level, areas);
			::resource_editor_window = new ResourceEditorWindow(resource_editor, areas, current_game_level, "1-1");
			
			::level_settings_window = new LevelSettingsWindow(level_redirect);
			
			resource_editor_window->status_bind(IO::status_handler);
			level_settings_window->status_bind(IO::status_handler);
		}

		std::string first_editor = construct_window();
		window->callback([](Fl_Widget* widget) {
			display_cb(false);
			flcb_force->value(0);
			level_forcer->set_enabled(false);
			window->hide();
		});

		if(!tp->valid()) {
			return false;
		}

		if(!level_forcer->valid()) {
			return false;
		}

		if(!level_redirect->valid()) {
			return false;
		}

		if(!scp->valid()) {
			return false;
		}

		if(!resource_editor->valid()) {
			return false;
		}

		construct_worker_thread();
		IO::NewFile();
		::window->redraw();

		if(!first_editor.empty()) {
			SetCurrentEditor(first_editor);
			initialized = true;
			return true;
		}
		else {
			return false;
		}
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
		return initialized;
	}

	std::shared_ptr<::Patch> GetPatch() {
		return tp;
	}
}