#pragma once

#include "winheaders.h"
#include <string>

namespace Registry {
	std::string GetValue(const std::string& key_name);
	void SetValue(const std::string& key_name, const std::string& value);
}