#pragma once

#include "winheaders.h"
#include <string>

#define BUF_SCAN_SIZE 4096

typedef DWORD Address;

namespace Signature {
	Address FindSignature(BYTE* signature, const std::string& mask, const Address start, const Address end, bool search_rewind, HANDLE my_proc);
}