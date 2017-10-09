#include "async_input_grab.h"

AsyncInputGrab::AsyncInputGrab(std::shared_ptr<InputReceivePatch> tirp, grab_fn tfn) 
	: irp(tirp), fn(tfn), kill_sig(false), input_count(0), flush(false)
{
	worker = std::thread([&]() {
		while(true) 
		{
			{
				kill_sig_mut.lock();
				if(kill_sig) {
					kill_sig_mut.unlock();
					return;
				}
				kill_sig_mut.unlock();
			}

			std::vector<InputFrame> frames;
			irp->pull_inputs(frames);

			if(flush) {
				flush = false;
				continue;
			}

			for(InputFrame frame : frames) {
				fn(frame);
				input_count++;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	});
}

AsyncInputGrab::~AsyncInputGrab() 
{
	kill_sig_mut.lock();
	kill_sig = true;
	kill_sig_mut.unlock();

	worker.join();
}

void AsyncInputGrab::query_flush_buffers() {
	flush = true;
}