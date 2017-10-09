#pragma once

#include <memory>

#include "derandom.h"
#include "game_hooks.h"
#include "input_frame.h"

//constructs a jmp-routine and generates an accessible pointer to it
//handles pushing inputs to memory and maintaining sanity between remote input ptr and local ptr
class InputPushBuilder {
private:
	bool is_valid;

	int input_buffer_position;
	int pid;

	Address cache_pos;

	std::shared_ptr<Spelunky> spel;
	std::shared_ptr<DerandomizePatch> dp;
	std::shared_ptr<GameHooks> gh;
	
	BYTE input_wait_buffer_size;

	Address subroutine;
	Address main_loop_end;
	Address input_wait_mode_ptr;
	Address froz_buffer_position_ptr;
	Address input_buffer_position_ptr;
	Address input_buffer_ptr;

	Address antihold_addr;
	BYTE* antihold_orig;
	Address antihold2_addr;
	BYTE* antihold2_orig;

public:
	InputPushBuilder(int p, std::shared_ptr<DerandomizePatch> dp, std::shared_ptr<GameHooks> gh);
	~InputPushBuilder();

	void perform();
	void undo();
	void destroy();
	bool valid();

	void flush();
	void push_input(const InputFrame& frame);
	int buffer_size();
	int get_pid();

	void set_cache_pos(Address var);
	Address get_cache_pos();

	size_t get_subroutine_size();
	Address get_subroutine();

	Address ibuf_position_ptr();
	void set_cancel_routine(Address jmpout);

	void write_rate_updater(Address rate);
};