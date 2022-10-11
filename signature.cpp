#pragma once

#include "winheaders.h"
#include <string>

#define BUF_SCAN_SIZE 4096

typedef DWORD Address;

namespace Signature {
	BYTE* _FindPattern(BYTE* buf, size_t datasize, BYTE* find, std::string mask, bool search_rewind=true) 
	{	
		int pos = 0;
		size_t size = mask.size();
		for(Address i = 0; i < datasize; i++) {
			if(mask[pos]=='.' || buf[i] == find[pos]) {
				pos++;
				if(pos == size) { 
					return &buf[i-pos+1];
				}
			}
			else {
				if(search_rewind)
					i -= pos;

				pos = 0;
			}
		}

		return NULL;
	}

	Address FindSignature(BYTE* signature, const std::string& mask, const Address start, const Address end, bool search_rewind, HANDLE my_proc) 
	{
		Address curr_addr = start;
		size_t mask_size = mask.size();

		BYTE buf[BUF_SCAN_SIZE];

		while(curr_addr < end) 
		{
			MEMORY_BASIC_INFORMATION mbi;
			if(!VirtualQueryEx(my_proc, reinterpret_cast<PVOID>(curr_addr), &mbi, sizeof(MEMORY_BASIC_INFORMATION))) {
				return NULL;
			}

			Address end = reinterpret_cast<Address>(mbi.BaseAddress) + mbi.RegionSize;
			size_t remainder = end - curr_addr;
			if(remainder > BUF_SCAN_SIZE) {
				remainder = BUF_SCAN_SIZE; 
			}

			size_t advance_by = remainder;
			if (remainder > mask_size) {
				advance_by -= mask_size;
			}
			
			if(mbi.State == MEM_COMMIT) 
			{
				if(!ReadProcessMemory(my_proc, reinterpret_cast<PVOID>(curr_addr), buf, remainder, NULL)) {
					curr_addr += advance_by;
					continue;
				}

				BYTE* ptr = _FindPattern(buf, remainder, signature, mask, search_rewind);
				Address actptr = (curr_addr+(ptr-buf));

				if(ptr != NULL) {
					return actptr;
				}
			}

			curr_addr += advance_by;
		}

		return NULL;
	}
}