#include "winheaders.h"

#include "updates.h"
#include "version.h"
#include "debug.h"

#include <boost/algorithm/string/trim.hpp>
#include <sstream>
#include <iostream>
#include <thread>
#include <curl/curl.h>

namespace Updates {
	result_callback_type res_callback;

	void UpdateChecker::open_update_page() {
		ShellExecute(NULL, "open", UPDATE_DOWNLOAD_URL, NULL, NULL, SW_SHOWNORMAL);
	}

	size_t update_available_callback(char* buf, size_t size, size_t count, void* up)
	{
		size_t handled = size*count;

		std::stringstream read;
		for(size_t i = 0; i < handled; i++) {
			read << buf[i];
		}

		std::string data = read.str();
		boost::algorithm::trim(data);

		if(data.size() < sizeof(VERSION_STR)-1 || data.size() >= 16) {
			res_callback(false, data);
			return handled;
		}
		else if(data.compare(VERSION_STR) != 0) {
			res_callback(true, data);
			return handled;
		}
		else {
			res_callback(false, data);
			return handled;
		}
		
#ifdef DEBUG_MODE
		std::cout << "Server version string: " << data << " (size " << data.size() << ")" << std::endl;
		std::cout << "Local version string: " << VERSION_STR << " (size " << sizeof(VERSION_STR)-1 << ")" << std::endl;
#endif

		return handled;
	}

	void UpdateChecker::update_available(result_callback_type result_callback) {
		res_callback = result_callback;

		CURL* curl = curl_easy_init();
		
		if(curl) 
		{
			curl_easy_setopt(curl, CURLOPT_URL, VERSION_CHECK_URL);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, update_available_callback);

#ifdef DEBUG_MODE
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#else
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
#endif

			curl_easy_perform(curl);
			curl_easy_cleanup(curl);
		}
	}


	std::function<void(bool)> easy_update_callback;

	void easy_update_check_thread() {
		UpdateChecker update;
		update.update_available([&update](bool result, const std::string& version_str) {
			if(result) {
				std::stringstream query;
				query << "New update available! ";
				query << "(" << version_str << ") ";
				query << "Download now?";
				std::string gen_query = query.str();

				if(MessageBox(NULL, gen_query.c_str(), "Frozlunky Updater", MB_YESNO) == IDYES) {
					update.open_update_page();
					exit(0);
				}

				easy_update_callback(true);
			}
			else {
				easy_update_callback(false);
			}
		});
	}

	void easy_update_check(std::function<void(bool)> result) {
		easy_update_callback = result;
		DWORD thread_id;
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)easy_update_check_thread, NULL, 0, &thread_id);
	}
}