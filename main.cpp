#define WINVER 0x05010000
#define _WIN32_WINNT 0x05010000

#include "winheaders.h"
#include "netplay_connection.h"

#include <string>
#include <memory>
#include <iostream>
#include <iomanip>

#include "gui.h"
#include "spelunky.h"
#include "debug.h"

#include <memory>
#include <curl/curl.h>


#ifndef DEBUG_MODE
int CALLBACK WinMain(
  _In_  HINSTANCE hInstance,
  _In_  HINSTANCE hPrevInstance,
  _In_  LPSTR lpCmdLine,
  _In_  int nCmdShow
)
#else
int main()
#endif
{
	curl_global_init(CURL_GLOBAL_ALL);
	atexit([]() {
		NetplayConnection::Deinitialize();
		undo_patches();
	});

	NetplayConnection::Initialize();
	
	_set_abort_behavior(0, _WRITE_ABORT_MSG);
	_set_abort_behavior(0, _CALL_REPORTFAULT);

	std::shared_ptr<Spelunky> spelunky = Spelunky::GetDefaultSpelunky();
	if(spelunky != nullptr) {
#ifdef DEBUG_MODE
		int val = gui_operate(spelunky, nullptr);
#else
		int val = gui_operate(spelunky, (char*)LoadIcon(hInstance, MAKEINTRESOURCE(101)));
#endif
		undo_patches();
		curl_global_cleanup();
		return val;
	}
	else {
		MessageBox(NULL, "Spelunky is not running, please start it and then launch Frozlunky. (Did you start Frozlunky as an Administrator?)", "Frozlunky", MB_OK);
		curl_global_cleanup();
		return 0;
	}
}