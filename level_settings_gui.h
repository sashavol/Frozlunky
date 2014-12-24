#pragma once

#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>

#include "level_forcer.h"
#include "cfn_util.h"

class LevelSettingsWindow : public Fl_Double_Window {
private:
	std::shared_ptr<LevelRedirect> redirect;
	CFnGen<void(Fl_Widget*)> cfn;
	std::function<void(int)> status_fn;

	Fl_Choice* level_starting;
	Fl_Choice* level_olmec;
	Fl_Choice* level_yama;
	Fl_Check_Button* checkpoints_enabled;

public:
	LevelSettingsWindow(std::shared_ptr<LevelRedirect> redirect);

	void update();
	void status_bind(std::function<void(int)> status_handler);
};