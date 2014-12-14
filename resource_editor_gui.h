#pragma once

#include <FL/Fl_Double_Window.H>

#include "resource_editor.h"
#include <memory>
#include <functional>
#include <string>

class ResourceEditorWindow : public Fl_Double_Window {
	std::shared_ptr<ResourceEditor> res_editor;
	std::function<std::string()> level_getter;

public:
	ResourceEditorWindow(std::shared_ptr<ResourceEditor> res, const std::vector<std::string>& areas, std::function<std::string()> level_getter, const std::string& first);
};