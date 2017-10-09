#pragma once

#include <memory>
#include <map>
#include <functional>
#include <iostream>
#include <iomanip>
#include <vector>

#include "signature.h"
#include "util.h"

int jne_diff(Address from, Address to);

inline int jmp_diff(Address from, Address to);
BYTE* jmp_opcode(Address from, Address to);

inline unsigned call_diff(Address from, Address to) {
	return to - (from+0x5);
}

inline unsigned je_diff(Address from, Address to) {
	return to - (from+0x6);
}

class Spelunky {
private:
	HANDLE process;
	std::multimap<Address, size_t> reserved;
	std::map<std::string, Address> hooks; 

public:
	Spelunky(HANDLE process);
	~Spelunky();

	Address get_stored_hook(const std::string& hook);
	void store_hook(const std::string& name, Address hook);

	bool alive();

	bool is_reserved(Address addr, size_t size);
	bool reserve(Address addr, size_t size);
	bool free(Address addr);
	
	template <class _Type>
	void read_mem(Address addr, _Type* dest, size_t len) {
		ReadProcessMemory(process, (LPCVOID)addr, dest, len, NULL);
	}

	void write_mem(Address addr, const void* value, size_t len, bool unlock_mem=false);

	Address find_mem(BYTE* signature, const std::string& mask, Address start = 0x0);
	Address find_exec_mem(BYTE* signature, const std::string& mask, Address start = 0x0);
	Address get_libc_func(const std::string& func);

	Address allocate(size_t bytes, bool execute=false);
	void release(Address addr);

	void jmp_build(Address from, size_t from_size, Address to, size_t to_size);

	std::string base_directory();

	static std::shared_ptr<Spelunky> GetDefaultSpelunky();

	friend class Patch;
};