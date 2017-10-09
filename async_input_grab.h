#pragma once

#include <thread>
#include <memory>
#include <functional>
#include <mutex>

#include "input_recv_patch.h"

class AsyncInputGrab {
public:
	typedef std::function<void(InputFrame)> grab_fn;

private:
	std::thread worker;
	std::shared_ptr<InputReceivePatch> irp;
	grab_fn fn;

	std::mutex kill_sig_mut;
	bool kill_sig;
	int input_count;
	bool flush;

public:
	AsyncInputGrab(std::shared_ptr<InputReceivePatch> irp, grab_fn fn);
	~AsyncInputGrab();

	void query_flush_buffers();
};