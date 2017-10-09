#include "../debug.h"
#include "request.h"

#include <curl/curl.h>
#include <map>
#include <sstream>
#include <mutex>
#include <iostream>

namespace Frozboards {
	extern "C"
	size_t write_fun(char* buf, size_t size, size_t count, void* ptr) 
	{
		Request* req = (Request*)ptr;
		std::string part(buf, size*count);
		(req->databuf) << part;
		return size*count;
	}

	Request::Request(std::string url, std::map<std::string, std::string> post) : url(url), post(post), started(false) {}
	Request::~Request() {}

	std::mutex connection_mutex;

	DWORD __stdcall request_thread(LPVOID param) 
	{
		connection_mutex.lock();

		Request* req = (Request*)param;

		CURL* curl;
		curl = curl_easy_init();
		
		if(curl) {
			curl_easy_setopt(curl, CURLOPT_URL, req->url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, req);
			
#ifdef DEBUG_MODE
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

			if(!req->post.empty()) {
				std::stringstream postss;

				for(std::pair<std::string, std::string> entry : req->post) {
					postss << entry.first << "=" << curl_easy_escape(curl, entry.second.c_str(), entry.second.size()) << "&";
				}

				std::string poststr = postss.str();
				poststr = poststr.substr(0, poststr.size()-1);
				size_t capacity = poststr.size();

				//gets deleted later at write callback or end of network thread
				char* mem = (char*)malloc(capacity);
				std::memcpy(mem, poststr.c_str(), capacity);

				curl_easy_setopt(curl, CURLOPT_HEADER, 0);
				curl_easy_setopt(curl, CURLOPT_POST, 1);
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, mem);
				curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, capacity);

				//TODO verify that mem is in fact disposed, the memory appears to be un-free-able after curl execution
			}

			curl_easy_setopt(curl, CURLOPT_WRITEDATA, req);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_fun);

			CURLcode res = curl_easy_perform(curl);
			if(res != CURLE_OK && req->ecb) {
				req->ecb(REQ_ERR_CONN_FAILURE);
			}

			//no response
			std::string data = req->databuf.str();
			if(data.empty()) {
				req->ecb(REQ_ERR_NO_RESPONSE);
			}
			else if(req->cb) {
				req->cb(data);
			}

			curl_easy_cleanup(curl);
		}

		connection_mutex.unlock();
		return 0;
	}

	void Request::perform(request_callback cb) {
		if(!started) {
			this->cb = cb;
			this->started = true;
			DWORD thread;
			CreateThread(NULL, 0, request_thread, this, NULL, &thread);
		}
	}
}