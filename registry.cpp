#include "registry.h"

namespace Registry {
	std::string GetValue(const std::string& key_name) {
		HKEY key;
		if(RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Frozlunky", NULL, KEY_READ, &key)) {
			return "";
		}
		else {
			char data[256];
			DWORD size = sizeof(data);
			DWORD dwType = REG_SZ;
			if(RegQueryValueEx(key, key_name.c_str(), 0, &dwType, (LPBYTE)data, &size) != ERROR_SUCCESS) {
				return "";
			}
			else {
				return std::string(data);
			}
		}
	}

	void SetValue(const std::string& key_name, const std::string& value) {
		HKEY key;
		RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Frozlunky", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, NULL);
		RegSetValueEx(key, key_name.c_str(), 0, REG_SZ, (const BYTE*)value.c_str(), value.size());
	}
}