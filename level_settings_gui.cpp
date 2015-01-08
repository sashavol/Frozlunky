#include "level_settings_gui.h"
#include "tile_editor_widget.h"
#include <map>
#include <boost/assign.hpp>

static void cfn_cb(Fl_Widget* o, void* fn) {
	(*static_cast<CFnGen<void(Fl_Widget*)>::heap_fn>(fn))(o);
}

static void fill_level_choices(Fl_Choice* choice) {
	for(int i = LEVEL_1_1; i <= LEVEL_5_4; ++i) {
		choice->add((std::to_string(1 + ((i-1)/4)) + "-" + std::to_string(1 + ((i-1)%4))).c_str());
	}
}

static int choice_level(int idx) {
	return 1 + idx;
}

void LevelSettingsWindow::inform() {
	//reset checkpoint
	redirect->checkpoint_mutex.lock();
	redirect->last_checkpoint = int(redirect->level_start);
	redirect->checkpoint_mutex.unlock();

	//signal update
	status_fn(STATE_CHUNK_WRITE);
}

LevelSettingsWindow::LevelSettingsWindow(std::shared_ptr<LevelRedirect> redirect) : 
	Fl_Double_Window(272, 218, "Level Settings"),
	redirect(redirect),
	status_fn([](int){})
{
	this->begin();
	{ 
		(new Fl_Button(10, 170, 255, 30, "Done"))->callback(cfn_cb, cfn.wrap([=](Fl_Widget*) {
			this->hide();
		}));
	} // Fl_Button* o
	{
		Fl_Group* o = new Fl_Group(10, 25, 255, 105, "Level Configuration");
		o->box(FL_UP_FRAME);
		
		{ 
			Fl_Choice* o = new Fl_Choice(115, 35, 140, 25, "Starting Level:");
			o->down_box(FL_BORDER_BOX);
			fill_level_choices(o);
			o->value(redirect->level_start - 1);

			o->callback(cfn_cb, cfn.wrap([=](Fl_Widget*) {
				redirect->level_start = choice_level(o->value());
				inform();
			}));

			level_starting = o;
		} // Fl_Choice* o

		{ 
			Fl_Choice* o = new Fl_Choice(115, 65, 140, 25, "Olmec Level:");
			o->down_box(FL_BORDER_BOX);
			fill_level_choices(o);
			o->value(redirect->level_olmec - 1);

			o->callback(cfn_cb, cfn.wrap([=](Fl_Widget*) {
				redirect->level_olmec = choice_level(o->value());
				inform();
			}));

			level_olmec = o;
		} // Fl_Choice* o
		
		{ 
			Fl_Choice* o = new Fl_Choice(115, 95, 140, 25, "Yama Level:");
			o->down_box(FL_BORDER_BOX);
			fill_level_choices(o);
			o->value(redirect->level_yama - 1);

			o->callback(cfn_cb, cfn.wrap([=](Fl_Widget*) {
				redirect->level_yama = choice_level(o->value());
				inform();
			}));

			level_yama = o;
		} // Fl_Choice* o
		
		o->end();
	} // Fl_Group* o
	{ Fl_Check_Button* o = new Fl_Check_Button(10, 136, 255, 25, "Quick Restart resets to current level");
		o->down_box(FL_DOWN_BOX);
		o->value(redirect->checkpoint_mode);

		o->callback(cfn_cb, cfn.wrap([=](Fl_Widget*) {
			redirect->checkpoint_mode = !!o->value();
			inform();
		}));

		checkpoints_enabled = o;
	} // Fl_Check_Button* o
	this->end();
}

void LevelSettingsWindow::status_bind(std::function<void(int)> status_handler) {
	status_fn = status_handler;
}

void LevelSettingsWindow::update() {
	level_starting->value(redirect->level_start - 1);
	level_olmec->value(redirect->level_olmec - 1);
	level_yama->value(redirect->level_yama  - 1);
	checkpoints_enabled->value(redirect->checkpoint_mode);
}
