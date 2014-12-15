#include "resource_editor_gui.h"

#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Spinner.H>

//TODO implement support for resource editor object modification
//TODO implement handling for data change, apply to resource editor.
//TODO implement handling for choice switch
//TODO implement Done button

//TODO custom enemy placement

ResourceEditorWindow::ResourceEditorWindow(std::shared_ptr<ResourceEditor> res, 
		const std::vector<std::string>& areas, 
		std::function<std::string()> level_getter, 
		const std::string& first) :
	Fl_Double_Window(198, 213, "Resource Editor"),
	res_editor(res),
	level_getter(level_getter)
{
	Fl_Double_Window* o = this;
	o->begin();

	{ Fl_Choice* o = new Fl_Choice(65, 15, 95, 25, "Level:");
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
	} // Fl_Choice* o
	{ new Fl_Button(10, 180, 180, 25, "Done");
	} // Fl_Button* o
	{ Fl_Spinner* o = new Fl_Spinner(65, 76, 90, 24, "Health:");
		o->minimum(-1);
		o->maximum(4);
		o->value(-1);
	} // Fl_Spinner* o
	{ Fl_Spinner* o = new Fl_Spinner(65, 106, 90, 24, "Bombs:");
		o->minimum(-1);
		o->maximum(4);
		o->value(-1);
	} // Fl_Spinner* o
	{ Fl_Spinner* o = new Fl_Spinner(65, 136, 90, 24, "Ropes:");
		o->minimum(-1);
		o->maximum(4);
		o->value(-1);
	} // Fl_Spinner* o
	{ Fl_Group* o = new Fl_Group(15, 65, 165, 10, "-1 = No change");
		o->end();
	} // Fl_Group* o

	o->end();
}