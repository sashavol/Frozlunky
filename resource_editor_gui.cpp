#include "resource_editor_gui.h"
#include "tile_editor_widget.h"

static void fn_callback(Fl_Widget* o, void* fn) {
	(*static_cast<ResourceEditorWindow::fn_gen::heap_fn>(fn))(o);
}

ResourceEditor::Resources& ResourceEditorWindow::get_res() {
	return res_editor->res(current_area);
}

ResourceEditorWindow::ResourceEditorWindow(std::shared_ptr<ResourceEditor> res, 
		const std::vector<std::string>& areas, 
		std::function<std::string()> level_getter, 
		const std::string& first) :
	Fl_Double_Window(198, 213, "Resource Editor"),
	res_editor(res),
	level_getter(level_getter),
	current_area(first),
	health_spinner(nullptr),
	bombs_spinner(nullptr),
	ropes_spinner(nullptr),
	area_choose(nullptr),
	status_handler([](int){})
{
	Fl_Double_Window* o = this;
	o->begin();

	{ Fl_Button* o = new Fl_Button(10, 180, 180, 25, "Done");
		o->callback(fn_callback, gen.wrap([=](Fl_Widget*) {
			this->hide();
		}));
	} // Fl_Button* o
	{ Fl_Choice* o = new Fl_Choice(65, 15, 95, 25, "Level:");
		area_choose = o;

		o->down_box(FL_BORDER_BOX);
		int idx = 0;
		int p = 0;
		for(auto&& area : areas) {
	 		o->add(area.c_str());
			if(area == first)
				idx = p;
			++p;
		}
		o->value(idx);
		
		o->callback(fn_callback, gen.wrap([=](Fl_Widget* o) {
			Fl_Choice* c = static_cast<Fl_Choice*>(o);
			int idx = c->value();
			if(idx >= 0) {
				current_area = areas[idx];
				ResourceEditor::Resources& res = get_res();
				bombs_spinner->value(res.bombs);
				health_spinner->value(res.health);
				ropes_spinner->value(res.ropes);
			}
			else {
				current_area = "";
			}
		}));
	} // Fl_Choice* o
	{ Fl_Spinner* o = new Fl_Spinner(65, 76, 90, 24, "Health:");
		health_spinner = o;

		o->minimum(MIN_HEALTH_VAL);
		o->maximum(MAX_HEALTH_VAL);
		o->value(-1);

		o->callback(fn_callback, gen.wrap([=](Fl_Widget*) {
			get_res().health = (int)health_spinner->value();
			status_handler(STATE_CHUNK_WRITE); //update
		}));

		//Currently disabled because health editing is unsupported
		health_spinner->deactivate();
	} // Fl_Spinner* o
	{ Fl_Spinner* o = new Fl_Spinner(65, 106, 90, 24, "Bombs:");
		bombs_spinner = o;

		o->minimum(MIN_BOMBS_VAL);
		o->maximum(MAX_BOMBS_VAL);
		o->value(-1);

		o->callback(fn_callback, gen.wrap([=](Fl_Widget*) {
			get_res().bombs = (int)bombs_spinner->value();
			status_handler(STATE_CHUNK_WRITE); //update
		}));
	} // Fl_Spinner* o
	{ Fl_Spinner* o = new Fl_Spinner(65, 136, 90, 24, "Ropes:");
		ropes_spinner = o;

		o->minimum(MIN_ROPES_VAL);
		o->maximum(MAX_ROPES_VAL);
		o->value(-1);

		o->callback(fn_callback, gen.wrap([=](Fl_Widget*) {
			get_res().ropes = (int)ropes_spinner->value();
			status_handler(STATE_CHUNK_WRITE); //update
		}));
	} // Fl_Spinner* o
	{ Fl_Group* o = new Fl_Group(15, 65, 165, 10, "-1 = No change");
		o->end();
	} // Fl_Group* o

	o->end();
}

void ResourceEditorWindow::update() {
	ResourceEditor::Resources& res = get_res();
	bombs_spinner->value(res.bombs);
	health_spinner->value(res.health);
	ropes_spinner->value(res.ropes);
}

void ResourceEditorWindow::status_bind(std::function<void(int)> status_handler) {
	this->status_handler = status_handler;
}