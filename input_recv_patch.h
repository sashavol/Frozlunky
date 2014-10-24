#pragma once

#include "patches.h"
#include "derandom.h"
#include "game_hooks.h"
#include "spelunky.h"
#include "input_frame.h"
#include "input_push_builder.h"

#include <vector>
#include <sstream>

bool state_accepts_input(int state);

/*
	Patch to store all real inputs to an infinite-index cyclic buffer spelunky-side.
	Abstracts syscalls to game to pull all collected inputs asynchronously.
*/
class InputReceivePatch : public Patch {
private:
	std::shared_ptr<GameHooks> gh;
	std::shared_ptr<DerandomizePatch> dp;
	std::shared_ptr<Spelunky> spel;
	std::shared_ptr<InputPushBuilder> ipb[2];

	bool is_valid;
	


	//++ input buffer patch
	Address exec_space;
	Address overwrite_addr;

	unsigned local_buffer_pos;
	Address input_buffer_position_ptr;
	Address input_buffer_ptr;


	BYTE* orig_jmpbuild_override;
	//==



	//++ dwi patch
	Address dwi;
	Address dwi_insert;
	Address dwi_exec_space;
	BYTE* orig_main_override;
	//==



	//++ input cycle block patch
	Address allow_input_cycle_ptr;
	Address input_cycle_fn;
	Address input_cycle_exec_space;
	BYTE* orig_input_cycle;
	//==



	//++ zero inputs patch-out
	Address zero_inputs_addr;
	BYTE* zero_inputs_orig;
	//==


	
	//++ input buffer position reset routine
	Address prev_ibufpos_addr;
	Address ibufpos_store_routine;
	Address ibufpos_reset_routine;
	Address main_loop_end;
	//==

	
	//++ player attribute tracking
	unsigned pid_attr_track;
	//==


	//++ push buffer position caching
	Address bufpos_cache_vars;
	Address bufpos_routine;
	//==

	//++ frame multiplier removal
	Address frame_mult_func;    //fn
	Address frame_mult_addr;    //14684D0
	Address frame_mult_fl_arb1; //14684C8
	Address frame_mult_in_arb2; //14684BC
	Address frame_mult_in_arb3; //14684B8
	BYTE* orig_frame_mult;
	//==

	//++ no mouse
	Address mouse_addr;
	BYTE* orig_mouse;
	//==

public:
	InputReceivePatch(std::shared_ptr<DerandomizePatch> dp, std::shared_ptr<GameHooks> gh);
	~InputReceivePatch();

private:
	bool perform_irp_patch();
	bool perform_persistent_dwi_patch();
	bool perform_input_cycle_patch();

	virtual bool _perform() override;
	virtual bool _undo() override;

public:
	virtual bool valid() override;

	//specify which pid's attributes to track (bombs, ropes, health, x pos, y pos)
	void set_pid_attributes_track(unsigned track);

	//slot = 0 or 1
	void set_input_push(int slot, std::shared_ptr<InputPushBuilder> ipb);

	//pulls inputs from internal buffer into target vector
	void pull_inputs(std::vector<InputFrame>& out);
};