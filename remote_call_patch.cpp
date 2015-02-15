#include "remote_call_patch.h"

#define CALL_INSERT_OVERRIDE_SIZE 6
//+0
BYTE call_insert_find[] = {0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0x6A,0xCC,0x6A,0xCC,0x6A,0xCC,0x6A,0xCC,0x8D,0xCC,0xCC,0xCC,0xCC,0xFF};
std::string call_insert_mask = "x.....x.x.x.x.x....x";

BYTE subroutine_footer[] = {0xC3};

BYTE wrapper_code[] = {0x60,0xA1,0x00,0x00,0x93,0x0C,0x83,0xF8,0x01,0x75,0x0C,0xE8,0x80,0x90,0xFE,0x83,0xC6,0x05,0x00,0x00,0x93,0x0C,0x00,0x61};

void RemoteCallPatch::init_worker() {
	worker = std::thread([=]() {
		bool accepting_new = true;
		std::function<void()> last_cb;

		while(true) 
		{
			if(destroy_thread) {
				return;
			}

			if(accepting_new) {
				queue_mutex.lock();
				bool avail = queue.size() > 0;
				queue_mutex.unlock();
			
				if(avail) {
					queue_mutex.lock();
					user_entry entry = queue[0];
					queue.erase(queue.begin());
					queue_mutex.unlock();

					//write function and store callback to be called when function executed
					last_cb = entry.second;
					spel->write_mem(routine_mem, entry.first.data, RCP_USER_SIZE);

					//signal to spelunky-side a new function call is ready
					unsigned tr = 1;
					spel->write_mem(var_mem, &tr, sizeof(unsigned));

					accepting_new = false;
				}
			}
			else {
				int response = 0;
				spel->read_mem(var_mem, &response, sizeof(int));

				if(!response) {
					if(last_cb) {
						last_cb();
					}
					accepting_new = true;
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			std::this_thread::yield();
		}
	});
}

RemoteCallPatch::RemoteCallPatch(std::shared_ptr<Spelunky> spel) : 
	Patch(spel),
	is_valid(true),
	call_insert_addr(0x0),
	routine_mem(0x0),
	var_mem(0x0),
	destroy_thread(false),
	thread_active(false),
	orig_ins(nullptr),
	routine_wrapper(0x0)
{
	routine_mem = spel->allocate(RCP_ROUTINE_ALLOC, true);
	if(!routine_mem) {
		DBG_EXPR(std::cout << "[RemoteCallPatch] Failed to allocate routine memory." << std::endl);
		is_valid = false;
		return;
	}
	DBG_EXPR(std::cout << "[RemoteCallPatch] Allocated routine_mem = " << routine_mem << std::endl);

	routine_wrapper = spel->allocate(RCP_WRAPPER_ALLOC, true);
	if(!routine_wrapper) {
		DBG_EXPR(std::cout << "[RemoteCallPatch] Failed to allocate routine_wrapper" << std::endl);
		is_valid = false;
		return;
	}
	DBG_EXPR(std::cout << "[RemoteCallPatch] Allocated routine_wrapper = " << routine_wrapper << std::endl);

	var_mem = spel->allocate(RCP_VAR_ALLOC);
	if(!var_mem) {
		DBG_EXPR(std::cout << "[RemoteCallPatch] Failed to allocate var_mem" << std::endl);
		is_valid = false;
		return;
	}
	DBG_EXPR(std::cout << "[RemoteCallPatch] Allocated var_mem = " << var_mem << std::endl);

	call_insert_addr = spel->find_mem(call_insert_find, call_insert_mask);
	if(!call_insert_addr) {
		DBG_EXPR(std::cout << "[RemoteCallPatch] Failed to find location of call_insert_addr" << std::endl);
		is_valid = false;
		return;
	}
	DBG_EXPR(std::cout << "[RemoteCallPatch] Located call_insert_addr = " << call_insert_addr << std::endl);

	orig_ins = new BYTE[CALL_INSERT_OVERRIDE_SIZE];
	spel->read_mem(call_insert_addr, orig_ins, CALL_INSERT_OVERRIDE_SIZE);

	init_worker();
}

RemoteCallPatch::~RemoteCallPatch() {
	destroy_thread = true;
	worker.join();

	if(routine_mem) {
		spel->free(routine_mem);
		routine_mem = 0;
	}

	if(var_mem) {
		spel->free(var_mem);
		var_mem = 0;
	}

	if(orig_ins) {
		delete[] orig_ins;
		orig_ins = nullptr;
	}
}

bool RemoteCallPatch::valid() {
	return is_valid;
}

bool RemoteCallPatch::enqueue_call(const BYTE* prefix, unsigned len, Address to, std::function<void()> cb) 
{
	//5 bytes for call
	if(len > RCP_USER_SIZE - 5) {
		return false;
	}
	
	UserCall u = {};
	std::fill(u.data, u.data + sizeof(u.data), 0x90); //fill with nops

	//write prefix
	std::memcpy(u.data, prefix, len);

	//write function call
	BYTE instr = 0xE8; //call
	std::memcpy(u.data+RCP_USER_SIZE-5, &instr, sizeof(BYTE));
	unsigned cdif = call_diff(routine_mem+RCP_USER_SIZE-5, to); 
	std::memcpy(u.data+RCP_USER_SIZE-4, &cdif, sizeof(unsigned));

	queue_mutex.lock();
	queue.push_back(user_entry(u, cb));
	queue_mutex.unlock();

	return true;
}

bool RemoteCallPatch::enqueue_call(std::shared_ptr<RemoteCallConstructor::RCData> data, std::function<void()> cb) {
	if(!data) {
		return false;
	}

	return enqueue_call(data->ctx, data->ctx_len, data->fn, cb);
}

bool RemoteCallPatch::_perform() {
	//init routine
	BYTE init[RCP_ROUTINE_ALLOC];
	std::fill(init, init+RCP_ROUTINE_ALLOC, 0x90); //fill with nop until worker thread writes call routine
	spel->write_mem(routine_mem, init, RCP_ROUTINE_ALLOC);
	spel->write_mem(routine_mem + RCP_USER_SIZE, subroutine_footer, sizeof(subroutine_footer));

	//init wrapper + jmp to wrapper
	spel->write_mem(routine_wrapper, wrapper_code, sizeof(wrapper_code));
	spel->write_mem(routine_wrapper + 0x01 + 1, &var_mem, sizeof(Address));
	signed diff = call_diff(routine_wrapper + 0xB, routine_mem);
	spel->write_mem(routine_wrapper + 0xB  + 1, &diff, sizeof(signed));
	spel->write_mem(routine_wrapper + 0x10 + 2, &var_mem, sizeof(Address));

	spel->jmp_build(call_insert_addr, CALL_INSERT_OVERRIDE_SIZE, routine_wrapper, sizeof(wrapper_code)); 
	return true;
}

bool RemoteCallPatch::_undo() {
	spel->write_mem(call_insert_addr, orig_ins, sizeof(wrapper_code));
	return true;
}