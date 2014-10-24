#pragma once

#include <string>

namespace Frozboards {
	std::string Encrypt(const std::string& plain);
	std::string Decrypt(std::string b64message);
}