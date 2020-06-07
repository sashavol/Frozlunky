#define MUPARSER_STATIC

#include "../debug.h"
#include "../winheaders.h"

#include "session.h"
#include "registry_token.h"
#include "crypt.h"
#include "unscrambler.h"
#include "request.h"

#include <mutex>
#include <curl/curl.h>
#include <muParser.h>
#include <boost/assign.hpp>

#define FROZ_AUTH_UI_URL "http://frozboards.sashavol.com/verify.htm?a="
#define FROZ_EQUATION_URL "http://frozboards.sashavol.com/__sys/calc.dat"

namespace Frozboards {
	BYTE req_sess_scramb[] = {150,137,136,139,192,214,215,145,132,154,142,145,157,144,130,
		139,157,195,159,138,153,129,137,145,137,137,202,128,141,140,207,128,129,174,165,168,
		245,171,189,166,137,166,177,160,161,184,191,161,224,189,164,187,202};
	BYTE req_submit_scramb[] = {150,137,136,139,192,214,215,145,132,154,142,145,157,144,130,
		139,157,195,159,138,153,129,137,145,137,137,202,128,141,140,207,128,129,174,165,168,
		245,170,173,181,137,165,189,163,183,189,185,161,171,227,188,163,186};
	BYTE req_error_scramb[] = {170,149,153,137,159,217,143,150,133,213,149,157,210,148,130,157,
		129,159,204,152,159,139,133,142,146,145,141,141,133,193,153,176,171,175,252,183,187,170,
		172,247,164,160,186,253,242,249,131,160,188,191,181,234,227,233,156,181,191,229,165,164,
		163,168,174,159,210,220,200,222,200,151};
	BYTE verification_scramb[] = {167,146,137,219,148,156,157,147,214,129,155,211,147,132,132,135,
		129,159,133,145,143,201,156,143,143,150,196,128,141,140,144,170,170,184,174,251,184,188,190,
		184,164,176,244,170,189,164,240,172,175,163,236,187,166,168,177,231,130,164,173,175,171,164,
		179,147,158,222,211,213,206,208,214,194,211,138};


#define UNSCRAMBLE_FUNC(FUNC_NAME, SCRAMB_NAME) \
	std::string FUNC_NAME() { \
		Unscrambler::membuf buf(SCRAMB_NAME, SCRAMB_NAME+sizeof(SCRAMB_NAME)); \
		{ \
			Unscrambler u(buf); \
			u.cycle_unscramble(); \
			buf = u.data(); \
			buf.push_back(0); \
		} \
		return std::string((char*)buf.data()); \
	}

	UNSCRAMBLE_FUNC(req_session_url, req_sess_scramb)
	UNSCRAMBLE_FUNC(req_submit_url, req_submit_scramb)
	UNSCRAMBLE_FUNC(req_error_message, req_error_scramb)
	UNSCRAMBLE_FUNC(req_verif_message, verification_scramb)


	void open_url(const std::string& url) {
		ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}


	/////////////
	//Dynamic score computation
	////////////

	std::mutex parser_mutex;
	mu::Parser* parser = nullptr;


	void Session::InitializeEquation(equ_init_callback res_cb) 
	{
		parser_mutex.lock();
		
		if(parser != nullptr) {
			delete parser;
			parser = nullptr;
		}

		Request* req = new Request(FROZ_EQUATION_URL, Request::post_map());

		req->set_error_callback([res_cb, req](int) {
			res_cb(false);
			delete req;
		});

		req->perform([req, res_cb](const std::string& result) {
			if(result == "") {
				res_cb(false);
				delete req;
				return;
			}

			parser_mutex.lock();
			parser = new mu::Parser();
			parser->SetExpr(result);
			parser_mutex.unlock();

			res_cb(true);
		
			delete req;
		});

		parser_mutex.unlock();
	}

	double point_pow(double base, double power) {
		return std::pow(base, power);
	}

	double Session::ComputePointScore(const Frozboards::TrackedValues& tv) {
		if(parser == nullptr) {
			return -1.0;
		}

		double L = tv.level, t = tv.level_time, T = tv.total_time;
		parser->DefineFun("pow", point_pow);
		parser->DefineVar("L", &L);
		parser->DefineVar("t", &t);
		parser->DefineVar("T", &T);

		return parser->Eval();
	}

	/////////////////////


	void Session::CreateSession(const std::string& sid, session_callback cb) 
	{
		if(parser == nullptr) {
			InitializeEquation([sid, cb](bool result) {
				if(result) {
					CreateSession(sid, cb);
				}
				else {
					cb(nullptr, SESSION_ERR_FAILED_REQ);
				}
			});
			return;
		}

		std::string auth_token = Registry::GetLoginToken();
		std::string data;
		if(auth_token != "") {
			data = Frozboards::Encrypt(std::string("---!!:")+sid+"-!-"+auth_token);
		}
		else {
			data = Frozboards::Encrypt(std::string("----!:")+sid);
		}

		Request* post = new Request(req_session_url(), boost::assign::map_list_of("D", data));
		post->set_error_callback([post, cb](int) {
			cb(nullptr, SESSION_ERR_FAILED_REQ);
			delete post;
		});

		post->perform([post, cb, sid](const std::string& data) {
			if(data != "") {
				std::string result = Frozboards::Decrypt(data);

				//authorization code result
				if(result.find("!----:") == 0) {
					if(MessageBox(NULL, req_verif_message().c_str(), "Frozboards", MB_YESNO) == IDYES) {
						open_url(std::string(FROZ_AUTH_UI_URL) + result.substr(6));
					}

					cb(nullptr, SESSION_VERIFYING_USER);
				}
				//session token result
				else if(result.find("!!!--:") == 0) {
					std::string data = result.substr(6);
					size_t parts[] = {data.find("-!-"), data.rfind("-!-")};

					std::string sess = data.substr(0, parts[0]);
					std::string auth = data.substr(parts[0]+3, parts[1]-(parts[0]+3));
					std::string seed = data.substr(parts[1]+3);
					Registry::SetLoginToken(auth);
					cb(std::make_shared<Frozboards::Session>(sid, sess, auth, seed), SESSION_SUCCESS);
				}
				else if(result.find("-----") == 0) {
					cb(nullptr, SESSION_ERR_ALREADY_PLAYED);
				}
				else {
					cb(nullptr, SESSION_ERR_FAILED_REQ);
				}
			}	

			delete post;
		});
	}
	



	DWORD __stdcall display_error(LPVOID) {
		MessageBox(NULL, req_error_message().c_str(), "Frozboards", MB_OK);
		return 0;
	}

	void create_display_error_thread() {
		DWORD thread;
		CreateThread(NULL, 0, display_error, NULL, NULL, &thread); 
	}

	void Session::submit(TrackedValuesSet& tvs, bool success, string_callback url_callback) 
	{
		std::string data = Frozboards::Encrypt(std::string("-!!!!:")+this->session_token+"-!-"+this->auth_token+"-!-"+EncodeMulti(tvs)+"-!-"+(success ? "1" : "0"));

		Request* post = new Request(req_submit_url(), boost::assign::map_list_of("D", data));
		post->set_error_callback([post](int fail) {
			create_display_error_thread();
			delete post;
		});

		post->perform([this, post, url_callback](const std::string& data) {
			std::string status = Frozboards::Decrypt(data);

			if(status.find("!!---:") == 0) { //session invalidated (url-!-ERR)
				this->valid = false;
				if(this->icb) {
					this->icb();
					this->icb = invalidate_callback();
				}
				
				std::string data = status.substr(6);
				if(url_callback) {
					url_callback(data.substr(0, data.find("-!-")));
				}
			}
			else if(status.find("!----:") == 0) {
				if(url_callback) {
					url_callback(status.substr(6));
				}
			}
			else {
				create_display_error_thread();
			}

			delete post;
		});

		runs_submitted++;
		if(runs_submitted >= SESSION_MAX_ATTEMPTS) {
			this->valid = false;
			if(this->icb) {
				this->icb();
				this->icb = invalidate_callback();
			}
		}
	}



}
