#include "spelunky.h"

Spelunky::Spelunky(HANDLE process) : process(process) {}

Spelunky::~Spelunky() {
	CloseHandle(process);
}

Address Spelunky::get_stored_hook(const std::string& hook) {
	auto iter = hooks.find(hook);
	if(iter != hooks.end()) {
		return iter->second;
	}
	else {
		return 0x0;
	}
}

void Spelunky::store_hook(const std::string& name, Address hook) {
	hooks[name] = hook;
}

bool Spelunky::alive() {
	DWORD result;
	GetExitCodeProcess(process, &result);
	return result == STILL_ACTIVE;
}

bool Spelunky::is_reserved(Address addr, size_t size) 
{
	if(reserved.find(addr) != reserved.end()) {
		return true;
	}

	Address faraddr = addr+size;
	for(std::pair<Address, size_t> res : reserved) {
		Address base = res.first;
		Address end = base+res.second;

		if((addr >= base && addr < end)
			|| (faraddr >= base && faraddr < end)) 
		{
			return true;
		}
	}

	return false;
}

bool Spelunky::reserve(Address addr, size_t size) {
	if(!is_reserved(addr, size)) {
		return true;
	}
	else {
		return false;
	}
}

bool Spelunky::free(Address addr) {
	auto iter = reserved.find(addr);
	if(iter != reserved.end()) {
		reserved.erase(iter);
		return true;
	}
	else {
		return false;
	}
}

void Spelunky::write_mem(Address addr, const void* value, size_t len, bool unlock_mem) {
	size_t write_len = len;

	if(unlock_mem) {
		DWORD oldrights;
		VirtualProtectEx(process, (LPVOID)addr, write_len, PAGE_EXECUTE_READWRITE, &oldrights);
	}

	WriteProcessMemory(process, (LPVOID)addr, value, write_len, NULL);
}

Address Spelunky::find_mem(BYTE* signature, const std::string& mask, Address start) {
	return Signature::FindSignature(signature, mask, start, 0x6000000, true, process);
}

Address Spelunky::find_exec_mem(BYTE* signature, const std::string& mask, Address start) 
{
	Address curr = start;
	MEMORY_BASIC_INFORMATION mbi;

	while(true) {
		curr = find_mem(signature, mask, curr+1);
		if(curr == 0x0) {
			return 0x0;
		}

		VirtualQueryEx(process, (LPCVOID)curr, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
		if(mbi.Protect & PAGE_EXECUTE_READ) {
			break;
		}
	}

	return curr;
}

Address Spelunky::get_libc_func(const std::string& func) {
	return (Address)util::GetRemoteProcAddress(process, "msvcrt.dll", func.c_str());
}

Address Spelunky::allocate(size_t bytes, bool execute) {
	return (Address)VirtualAllocEx(process, NULL, bytes, MEM_COMMIT | MEM_RESERVE, (execute ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE));
}

void Spelunky::release(Address addr) {
	VirtualFreeEx(process, (LPVOID)addr, 0, MEM_RELEASE); 
}

int jne_diff(Address from, Address to) {
	return (to - (from + 0x6));
}

int jmp_diff(Address from, Address to) {
	return (to - (from + 0x5));
}

BYTE* jmp_opcode(Address from, Address to) {
	BYTE* jmp = new BYTE[5];

	int diff = jmp_diff(from, to);
	jmp[0] = 0xE9;
	std::memcpy(jmp+1, &diff, sizeof(int));

	return jmp;
}

void Spelunky::jmp_build(Address from, size_t from_size, Address to, size_t to_size) {
	BYTE* overwrite_code = new BYTE[from_size];
	BYTE* jmpto = jmp_opcode(from, to);
	std::memcpy(overwrite_code, jmpto, 5);
	delete[] jmpto;

	//fill the remainder with NOPs
	size_t remainder = from_size - 5;
	for(size_t i = 0; i < remainder; i++) {
		overwrite_code[i+5] = 0x90;
	}

	Address jmpback_addr = to+to_size;

	BYTE* jmpback_code = new BYTE[from_size+5];
	read_mem(from, jmpback_code, from_size);

	//OPT this does not check if an instruction is being overwritten, just bytes.
		//may be worth fixing
	//realign jmps and calls
	for(unsigned i = 0; i < from_size;) {
		if(jmpback_code[i] == 0xE8 || jmpback_code[i] == 0xE9) {
			signed int diff;
			std::memcpy(&diff, jmpback_code + i + 1, sizeof(signed int));
			diff += from - jmpback_addr;
			std::memcpy(jmpback_code + i + 1, &diff, sizeof(signed int));
			i += 5;
		}
		else {
			break;
		}
	}

	BYTE* jmpback = jmp_opcode(jmpback_addr+from_size, from+from_size);
	std::memcpy(jmpback_code+from_size, jmpback, 5);
	delete[] jmpback;

	write_mem(jmpback_addr, jmpback_code, from_size+5);
	write_mem(from, overwrite_code, from_size);

	delete[] overwrite_code;
	delete[] jmpback_code;
}



std::string Spelunky::base_directory() {
	char buffer[MAX_PATH];
	GetModuleFileNameEx(process, NULL, buffer, MAX_PATH);

	std::string str(buffer);
	return str.substr(0, str.find_last_of("\\/")) + "\\";
}


std::string GetLastErrorAsString()
	{
		//Get the error message, if any.
		DWORD errorMessageID = ::GetLastError();
		if(errorMessageID == 0)
			return "No error message has been recorded";

		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
									 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		std::string message(messageBuffer, size);

		//Free the buffer.
		LocalFree(messageBuffer);

		return message;
	}

std::shared_ptr<Spelunky> Spelunky::GetDefaultSpelunky() {
	DWORD spelunky_pid = util::FindProcess("Spelunky.exe");

	if(spelunky_pid != 0) {
		HANDLE handle = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION|PROCESS_SET_INFORMATION|PROCESS_SET_QUOTA|PROCESS_TERMINATE|PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE,
			TRUE, spelunky_pid);

		if(!handle) {
			MessageBox(NULL, (std::string("Unable to access Spelunky process: ")+GetLastErrorAsString()).c_str(), "Error", MB_OK);
			return nullptr;
		}
		return std::make_shared<Spelunky>(handle);
	}
	else {
		return nullptr;
	}
}