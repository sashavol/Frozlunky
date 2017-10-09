#pragma once

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Spinner.H>

#include "cfn_util.h"
#include "resource_editor.h"
#include <memory>
#include <functional>
#include <string>

class ResourceEditorWindow : public Fl_Double_Window {
public:
	typedef CFnGen<void(Fl_Widget*)> fn_gen;

private:
	std::function<void(int)> status_handler;

	std::shared_ptr<ResourceEditor> res_editor;
	std::function<std::string()> level_getter;
	fn_gen gen;

	Fl_Choice* area_choose;
	Fl_Spinner* health_spinner;
	Fl_Spinner* bombs_spinner;
	Fl_Spinner* ropes_spinner;

	std::string current_area;

private:
	ResourceEditor::Resources& get_res();

public:
	ResourceEditorWindow(std::shared_ptr<ResourceEditor> res, const std::vector<std::string>& areas, std::function<std::string()> level_getter, const std::string& first);
	
	void status_bind(std::function<void(int)> status_handler);
	void update();
};