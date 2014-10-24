#pragma once

#include <map>
#include <string>
#include <sstream>
#include <functional>

#define REQ_ERR_NO_RESPONSE 1
#define REQ_ERR_CONN_FAILURE 2

namespace Frozboards {
	class Request {
	public:
		typedef std::function<void(const std::string&)> request_callback;
		typedef std::function<void(int)> error_callback;
		typedef std::map<std::string, std::string> post_map;

	public:
		std::map<std::string, std::string> post;
		std::string url;
		std::stringstream databuf;
		request_callback cb;
		error_callback ecb;
		bool started;

	public:
		Request(std::string url, post_map posts);
		~Request();

		void perform(request_callback cb);
		void set_error_callback(error_callback ecb) {
			this->ecb = ecb;
		}
	};
}