#pragma once

#include "mods.h"
#include "all_dark.h"
#include "dark_possible.h"
#include "append_ai.h"
#include "timer99.h"
#include "precise_timer.h"

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Group.H>


namespace Mods {
	std::shared_ptr<PatchGroup> mods;

	visibility_callback visible_callback;
	Fl_Double_Window* mods_window = nullptr;

	std::map<int, unsigned> choice_entity_map;


	int DoneButton::handle(int evt) {
		if(evt == 2) {
			mods_window->hide();
			visible_callback(false);
		}
		return Fl_Button::handle(evt);
	}

	
	void ModCheckbox_callback(Fl_Widget* widget) {
		ModCheckbox* check = dynamic_cast<ModCheckbox*>(widget);
		std::shared_ptr<Patch> patch = mods->get(check->target);
		if(patch && !patch->locked()) {
			if(patch->is_active()) {
				patch->undo();
				check->value(0);
			}
			else {
				patch->perform();
				check->value(1);
			}
		}
	}

	ModCheckbox::ModCheckbox(const std::string& target, int x, int y, int w, int h, const char* label) 
		: Fl_Check_Button(x,y,w,h,label), target(target)
	{
		this->callback(ModCheckbox_callback);
		std::shared_ptr<Patch> patch = mods->get(target);
		if(patch && patch->valid()) {
			this->value(patch->is_active());
		}
		else {
			this->deactivate();
		}
	}


	void ShopItemCheckbox_callback(Fl_Widget* widget) {
		ShopItemCheckbox* sic = dynamic_cast<ShopItemCheckbox*>(widget);
		ItemID item = sic->id;
		
		std::shared_ptr<ShopContentsPatch> som = std::dynamic_pointer_cast<ShopContentsPatch>(mods->get("smo"));
		ShopContentsPatch::item_storage items = som->get_items();
		
		if(sic->value()) {
			items.push_back(item);
		}
		else {
			auto item_iter = std::find(items.begin(), items.end(), item);
			if(item_iter != items.end()) {
				items.erase(item_iter);
			}
		}

		som->set_items(items);
		if(items.size() > 0) {
			som->perform();
		}
		else {
			som->undo();
		}
	}

	ShopItemCheckbox::ShopItemCheckbox(ItemID item, int x, int y, int w, int h, const char* label)
		: Fl_Check_Button(x,y,w,h,label), id(item)
	{
		this->callback(ShopItemCheckbox_callback);
		std::shared_ptr<Patch> patch = mods->get("smo");
		if(patch && patch->valid()) {
			std::shared_ptr<ShopContentsPatch> som = std::dynamic_pointer_cast<ShopContentsPatch>(patch);
			ShopContentsPatch::item_storage items = som->get_items();
			this->value(std::find(items.begin(), items.end(), item) != items.end());
		}
		else {
			this->deactivate();
		}
	}

	void shop_only12_callback(Fl_Widget* widget) {
		Fl_Check_Button* check = dynamic_cast<Fl_Check_Button*>(widget);
		std::shared_ptr<ShopContentsPatch> som = std::dynamic_pointer_cast<ShopContentsPatch>(mods->get("smo"));
		
		if(check->value()) {
			som->set_levels(4);
		}
		else {
			som->set_levels(0xFFFFFFFF);
		}
	}


	Fl_Tree* appended_ai_tree = nullptr;
	Fl_Choice* entity_list = nullptr;

	int RemoveAIButton::handle(int evt) {
		if(evt == 2) {
			if(appended_ai_tree) 
			{
				std::shared_ptr<AppendAIPatch> aip = std::dynamic_pointer_cast<AppendAIPatch>(mods->get("aip"));
				Fl_Tree_Item* first = appended_ai_tree->first();

				if(first->is_selected()) {
					for(int child = 0; child < first->children(); child++) {
						delete (int*)(first->child(child)->user_data());
					}
					first->clear_children();
					aip->undo();
					aip->set_appended_ai(AppendAIPatch::entity_storage());
					mods_window->redraw();
					return Fl_Button::handle(evt);
				}

				for(int pos = 0; pos < first->children(); pos++)
				{
					if(!first->child(pos)->is_selected()) {
						continue;
					}

					Fl_Tree_Item* selected = first->child(pos);

					int appended_idx = *(int*)selected->user_data();

					//correct indices after removal
					for(int child = 0; child < first->children(); child++) {
						int* idx = (int*)(first->child(child)->user_data());
						if(*idx > appended_idx) {
							(*idx)--;
						}
					}

					AppendAIPatch::entity_storage appended = aip->appended_ai();
					appended.erase(appended.begin()+appended_idx);
					aip->set_appended_ai(appended);
					
					delete (int*)(selected->user_data());
					appended_ai_tree->remove(selected);
				}

				mods_window->redraw();
				if(aip->appended_ai().empty()) {
					aip->undo();
				}
			}
		}

		return Fl_Button::handle(evt);
	}

	int AddAIButton::handle(int evt) {
		if(evt == 2) {
			std::shared_ptr<AppendAIPatch> aip = std::dynamic_pointer_cast<AppendAIPatch>(mods->get("aip"));
			AppendAIPatch::entity_storage appended = aip->appended_ai();
			unsigned added = choice_entity_map[entity_list->value()];
			appended.push_back(added);
			aip->set_appended_ai(appended);

			appended_ai_tree->first()->add(Fl_Tree_Prefs(), AppendAIPatch::FriendlyName(added).c_str())->user_data(new int(appended.size()-1));

			if(!aip->appended_ai().empty()) {
				aip->perform();
			}
			mods_window->redraw();
		}
		return Fl_Button::handle(evt);
	}

	void init_appendai_list() {
		std::shared_ptr<AppendAIPatch> aip =  std::dynamic_pointer_cast<AppendAIPatch>(mods->get("aip"));

		Fl_Tree* tree = new Fl_Tree(5, 70+40, 254, 70, "Hybrid Enemies Mod");
		if(!aip->valid()) {
			tree->deactivate();
			return;
		}

		Fl_Tree_Item* first = tree->first();
		first->label("Enemy AI");

		AppendAIPatch::entity_storage appended_ais = aip->appended_ai();
		int idx = 0;
		for(unsigned ai : appended_ais) {
			first->add(Fl_Tree_Prefs(), AppendAIPatch::FriendlyName(ai).c_str())->user_data(new int(idx));
			idx++;
		}

		new AddAIButton(150, 145+40, 35, 20, "Add");
		new RemoveAIButton(190, 145+40, 70, 20, "Remove");

		appended_ai_tree = tree;
	}

	void init_appendai_choices() {
		Fl_Choice* o = new Fl_Choice(5, 145+40, 140, 20, "");
		o->down_box(FL_DOWN_BOX);
		std::shared_ptr<AppendAIPatch> aip = std::dynamic_pointer_cast<AppendAIPatch>(mods->get("aip"));

		if(!aip->valid()) {
			o->deactivate();
		}
		else {
			std::map<std::string, unsigned> order;
			auto entities = aip->entities();

			{
				int idx = 0;
				for(unsigned entity : entities) {
					order[AppendAIPatch::FriendlyName(entity)] = entity;
					idx++;
				}

				if(choice_entity_map.empty()) {
					idx = 0;
					for(std::pair<std::string, unsigned> pair : order) {
						choice_entity_map[idx] = pair.second;
						idx++;
					}
				}

				for(std::pair<std::string, unsigned> pair : order) {
					o->add(pair.first.c_str());
				}
			}

			o->value(o->find_item(order.begin()->first.c_str()));
		}

		entity_list = o;
	}

	void unstable_check_callback(Fl_Widget* widget) {
		std::shared_ptr<AppendAIPatch> aip = std::dynamic_pointer_cast<AppendAIPatch>(mods->get("aip"));
		Fl_Check_Button* unstable = dynamic_cast<Fl_Check_Button*>(widget);
		if(unstable->value()) {
			AppendAIPatch::entity_storage stor;
			stor.push_back(ENTITY_PIRANHA);
			stor.push_back(ENTITY_FISH_CRITTER);
			stor.push_back(ENTITY_PIRANHA_CRITTER);
			aip->set_ignored_entities(stor);
		}
		else {
			aip->set_ignored_entities(AppendAIPatch::entity_storage());
		}
	}

	Fl_Double_Window* make_mods_window() {
		if(mods_window != nullptr) {
			delete (Fl_Double_Window*)mods_window;
			mods_window = nullptr;
		}

		std::shared_ptr<AppendAIPatch> aip = std::dynamic_pointer_cast<AppendAIPatch>(mods->get("aip"));

		Fl_Double_Window* w;
		{ Fl_Double_Window* o = new Fl_Double_Window(264, 340, "Special Mods");
		w = o;
		{ new DoneButton(5, 310, 255, 25, "Close Window");
		} // Fl_Button* o
		{ Fl_Check_Button* o = new ModCheckbox("dpos", 5, 10, 255, 15, "Dark ice caves and hell are possible");
		o->down_box(FL_DOWN_BOX);
		} // Fl_Check_Button* o
		{ Fl_Check_Button* o = new ModCheckbox("alld", 5, 30, 255, 15, "All levels are dark");
		o->down_box(FL_DOWN_BOX);
		} // Fl_Check_Button* o
		{ Fl_Check_Button* o = new ModCheckbox("smlt", 5, 50, 255, 15, "End Timer is not set to 99 minutes");
		  o->down_box(FL_DOWN_BOX);
		}
		{ Fl_Check_Button* o = new ModCheckbox("pret", 5, 70, 255, 15, "Display precise timer");
		  o->down_box(FL_DOWN_BOX);
		}
		{
			{ 
				init_appendai_choices();
				init_appendai_list();
			}
			{ Fl_Check_Button* o = new Fl_Check_Button(5, 172+40, 255, 15, "Piranhas unaffected (anti-crash)");
			  o->callback(unstable_check_callback);
			  if(aip->ignored_entities().size() > 0) {
				o->value(true);
			  }
			  else {
				o->value(false);
			  }
			}
		}

		std::shared_ptr<ShopContentsPatch> som = std::dynamic_pointer_cast<ShopContentsPatch>(mods->get("smo"));

		{ Fl_Group* o = new Fl_Group(5, 190+25+40, 255, 45, "Shop Mods");
		{ Fl_Check_Button* o = new ShopItemCheckbox(ITEM_JETPACK, 10, 195+25+40, 75, 15, "Jetpack");
			o->down_box(FL_DOWN_BOX);
		  } // Fl_Check_Button* o
		  { Fl_Check_Button* o = new ShopItemCheckbox(ITEM_TELEPORTER, 85, 195+25+40, 90, 15, "Teleporter");
			o->down_box(FL_DOWN_BOX);
		  } // Fl_Check_Button* o
		  { Fl_Check_Button* o = new ShopItemCheckbox(ITEM_COMPASS, 175, 195+25+40, 83, 15, "Compass");
			o->down_box(FL_DOWN_BOX);
		  } // Fl_Check_Button* o
		  { Fl_Check_Button* o = new ShopItemCheckbox(ITEM_MATTOCK, 10, 215+25+40, 85, 15, "Mattock");
		    o->down_box(FL_DOWN_BOX);
		  }
		  { Fl_Check_Button* o = new ShopItemCheckbox(ITEM_BOMB_BOX, 85, 215+25+40, 110, 15, "Bomb Box");
		    o->down_box(FL_DOWN_BOX);
		  }
		  { Fl_Check_Button* o = new Fl_Check_Button(175, 215+25+40, 110, 15, "1-2 Only");
			o->down_box(FL_DOWN_BOX);
			if(som->valid()) {
				o->value(som->levels() == 4);
				o->callback(shop_only12_callback);
			}
			else {
				o->deactivate();
			}
		  } // Fl_Check_Button* o
		  o->box(FL_UP_FRAME);
		  o->end();
			
		} // Fl_Group* o


		o->end();
		} // Fl_Double_Window* o

		mods_window = w;

		return w;
	}

	void Initialize(std::shared_ptr<DerandomizePatch> dp, std::shared_ptr<GameHooks> gh, std::shared_ptr<CustomHudPatch> chp) {
		std::shared_ptr<Spelunky> spel = dp->spel;
		mods = std::make_shared<PatchGroup>(spel);
		mods->add("alld", std::make_shared<AllDarkPatch>(spel));
		mods->add("dpos", std::make_shared<DarkPossiblePatch>(spel));
		mods->add("aip", std::make_shared<AppendAIPatch>(spel));
		mods->add("smo", std::make_shared<ShopContentsPatch>(dp));
		mods->add("smlt", std::make_shared<Timer99Patch>(dp));
		mods->add("pret", std::make_shared<PreciseTimerPatch>(dp, gh,  chp));
	}

	std::shared_ptr<PatchGroup> ModsGroup() {
		return mods;
	}

	void ShowModsGUI() {
		Fl_Double_Window* wind = make_mods_window();

		wind->callback([](Fl_Widget* window) {
			if(visible_callback) {
				visible_callback(false);
			}

			if(window) {
				delete (Fl_Double_Window*)window;
				mods_window = nullptr;
			}

			if(appended_ai_tree) {
				Fl_Tree_Item* first = appended_ai_tree->first();
				if(first) {
					for(int i = 0; i < first->children(); i++) {
						delete (int*)(first->child(i)->user_data());
					}
				}
				appended_ai_tree = nullptr;
			}
		});
		wind->show();

		visible_callback(true);
	}

	void HideModsGUI() {
		if(visible_callback) {
			visible_callback(false);
		}
		
		if(mods_window) {
			mods_window->hide();
			delete mods_window;
			mods_window = nullptr;
		}
	}

	void SetVisibilityChangeCallback(visibility_callback cb) {
		visible_callback = cb;
	}
}