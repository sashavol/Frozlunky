#pragma once

#include <string>
#include <functional>

#define VERSION_CHECK_URL "http://sashavol.com/frozlunky/version.dat"
#define UPDATE_DOWNLOAD_URL "http://sashavol.com/frozlunky"

namespace Updates {
	typedef std::function<void(bool, const std::string&)> result_callback_type;

	class UpdateChecker {
	public:
		UpdateChecker() {}

		void update_available(result_callback_type result_callback);
		void open_update_page();
	};

	void easy_update_check(std::function<void(bool)> result_callback);
}