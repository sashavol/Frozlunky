#pragma once

#include "patches.h"
#include "rc.h"

#include <functional>
#include <vector>
#include <thread>
#include <mutex>

#define RCP_VAR_ALLOC 8
#define RCP_WRAPPER_ALLOC 256
#define RCP_ROUTINE_ALLOC 128
#define RCP_USER_SIZE (RCP_ROUTINE_ALLOC-32)

class RemoteCallPatch : public Patch {
private:
	struct UserCall {
		BYTE data[RCP_USER_SIZE];
	};

	typedef std::pair<UserCall, std::function<void()>> user_entry;

private:
	bool destroy_thread;
	bool thread_active;
	std::thread worker;

	bool is_valid;
	
	Address call_insert_addr;
	Address routine_mem;
	Address var_mem;
	Address routine_wrapper;

	BYTE* orig_ins;

	std::vector<user_entry> queue;
	std::mutex queue_mutex;

public:
	RemoteCallPatch(std::shared_ptr<Spelunky> spel);
	~RemoteCallPatch();

	bool enqueue_call(const BYTE* prefix, unsigned len, Address fn, std::function<void()> cb = std::function<void()>());
	bool enqueue_call(std::shared_ptr<RemoteCallConstructor::RCData> data, std::function<void()> cb = std::function<void()>());

private:
	void init_worker();
	
	bool _perform() override;
	bool _undo() override;

public:
	bool valid() override;
};