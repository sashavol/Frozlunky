#pragma once

#include <string>

/*
	General-purpose random phonetic string generation.
	Implemented based on Urbit-style phonetic token generation.
*/
namespace Syllabic {
	//phonetic string from seed
	std::string MakePhoneticString(unsigned seed, int parts);

	//random phonetic string
	std::string MakePhoneticString(int parts);
}