#pragma once

#include "../winheaders.h"
#include <string>

namespace Frozboards {
	namespace Registry {
		std::string GetLoginToken() {
			HKEY key;
			if(RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Frozlunky", NULL, KEY_READ, &key)) {
				return "";
			}
			else {
				char data[256];
				DWORD size = sizeof(data);
				DWORD dwType = REG_SZ;
				RegQueryValueEx(key, "Token", 0, &dwType, (LPBYTE)data, &size);
				return std::string(data);
			}
		}

		void SetLoginToken(const std::string& token) {
			HKEY key;
			RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Frozlunky", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, NULL);
			RegSetValueEx(key, "Token", 0, REG_SZ, (const BYTE*)token.c_str(), token.size());
		}
	}
}