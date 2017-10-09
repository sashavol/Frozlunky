#include "input_recv_patch.h"

#define INPUT_BUFFER_REMOTE_SIZE (INPUT_FRAME_REMOTE_SIZE*8192)

int states_accepting_input[] = {23, 26, 22, 11, 2, 3, 17, 30, 18, 31};

bool state_accepts_input(int state) {
	return std::find(states_accepting_input, states_accepting_input+sizeof(states_accepting_input)/sizeof(int), state) >= 0;
}



///////////////////////
//INPUT CYCLE PATCH  //
///////////////////////



#define EXEC_SPACE_ALLOC_SIZE 512
#define RECV_PATCH_OVERWRITE_SIZE 5
//+0
BYTE recv_patch_overwrite_find[] = {0x83,0xFF,0xFF,0xFF,0x03,0x75,0xFF,0xC6,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x0F,0xFF,0xFF,0x88};
std::string recv_patch_overwrite_mask = "x...xx.x......x..x";

BYTE recv_patch_opcodes[] = {
	0x52,0x53,0x50,0x81,0xF9,0xCC,0xCC,0xCC,0xCC,
	 
	0x0F,0x8F,0xCC,0xCC,0xCC,0xCC,

	0x51,0x31,0xD2,0xA1,0xCC,0xCC,0xCC,0xCC,
	
	0xBB,0xCC,0xCC,0xCC,0xCC,
	
	0xF7,0xF3,0x59,0x8B,0xC2,0xBA,0xCC,0xCC,0xCC,0xCC,
	
	0xF7,0xE2,0x05,0xCC,0xCC,0xCC,0xCC,
	
	0x8B,0xD8,0xC6,0x43,0x21,0x00,0x8D,0x04,0x8D,0xCC,0xCC,0xCC,0xCC,
	
	0x8A,0x00,0x88,0x03,0x8A,0x06,0x88,0x43,0x01,0x8A,
	0x46,0x01,0x88,0x43,0x02,0x8A,0x46,0x02,0x88,0x43,
	0x03,0x8A,0x46,0x03,0x88,0x43,0x04,0x8A,0x46,0x05,
	0x88,0x43,0x05,0x8A,0x46,0x0D,0x88,0x43,0x06,0x8B,
	0x46,0x1C,0x89,0x43,0x07,0x8B,0x46,0x20,0x89,0x43,0x0B,
	0x8A,0x46,0x04,0x88,0x43,0x0F,
	0x8A,0x46,0x06,0x88,0x43,0x10,
	0x8A,0x46,0x07,0x88,0x43,0x11,
	0x8A,0x46,0x08,0x88,0x43,0x12,
	0x8A,0x46,0x09,0x88,0x43,0x13,
	0x8A,0x46,0x0A,0x88,0x43,0x14,
	0x8A,0x46,0x0B,0x88,0x43,0x15,
	0x8A,0x46,0x0E,0x88,0x43,0x16,
	0x8A,0x46,0x0F,0x88,0x43,0x17,
	0x8A,0x46,0x10,0x88,0x43,0x18,
	0x8A,0x46,0x11,0x88,0x43,0x19,
	0x8A,0x46,0x12,0x88,0x43,0x1A,
	0x8A,0x46,0x13,0x88,0x43,0x1B,
	0x8A,0x46,0x14,0x88,0x43,0x1C,
	0x8A,0x46,0x15,0x88,0x43,0x23,
	0x8A,0x46,0x0C,0x88,0x43,0x24,
	0x8A,0x46,0x16,0x88,0x43,0x25,
	0x8A,0x46,0x17,0x88,0x43,0x26,
	0x8A,0x46,0x18,0x88,0x43,0x27,
	0x8A,0x46,0x19,0x88,0x43,0x28,
	0x8A,0x46,0x1A,0x88,0x43,0x29,
	0x8A,0x46,0x1B,0x88,0x43,0x2A,
	0xA1,0xCC,0xCC,0xCC,0xCC,
	
	0x89,0x43,0x1D,0xC6,0x43,0x21,0x01,0xC6,0x43,0x22,0x00,0xA1,0xCC,0xCC,0xCC,0xCC,
	
	0x40,0x89,0x05,0xCC,0xCC,0xCC,0xCC,

	0x58,0x5B,0x5A
};




////////////////////////
//PERSISTENT DWI PATCH//
////////////////////////

//+0
//BYTE dwi_find[] = {0x83,0xFF,0xFF,0xD9,0xFF,0xFF,0xD9,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x33,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x8D,0xFF,0xFF,0x69};
//std::string dwi_mask = "x..x..x......x......x..x";
BYTE dwi_find[] = {0x81,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x80,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x74,0xFF,0xE8,0xFF,0xFF,0xFF,0xFF,0x80};
std::string dwi_mask = "x.........x......x.x....x";


//OPT change the dwi insert to support the are you sure menu in the pause menu

//#define DWI_OVERRIDE_SIZE 5
//+0
//BYTE dwi_insert_find[] = {0xE8,0xCC,0xCC,0xCC,0xCC,0xE8,0xCC,0xCC,0xCC,0xCC,0x6A,0x01,0xFF,0xCC,0xCC,0xCC,0xCC,0xCC,0xE9};
//std::string dwi_insert_mask = "x....x....xxx.....x";

#define DWI_OVERRIDE_SIZE 6
BYTE dwi_insert_find[] = {0xDD,0xCC,0xCC,0xCC,0xCC,0xCC,0x83,0xCC,0xCC,0xDD,0xCC,0xCC,0xCC,0xCC,0xCC,0xD8,0xCC,0xDD,0xCC,0xCC,0xCC,0xCC,0xCC,0xDD,0xCC,0xCC,0xE8};
std::string dwi_insert_mask = "x.....x..x.....x.x.....x..x";

//!TODO make dwi_insert_oper a context setup for the jmp to irp exec. (player idx => ecx, controller => esi, etc)
																			//OPT 8B 46 40 = mov eax, [esi+0x40]; 40 should be a dynamic replacement for future sake. 
																			//definitely check this if there's spontaneous breaks in the future
BYTE dwi_insert_oper[] = {0x50,0x52,0x53,0x51,0x56,0x57,
	0x8B,0x35,0xAA,0xAA,0xAA,0xAA,
	0xFF,0x05,0xBB,0xBB,0xBB,0xBB,
	0x8B,0x46,0x40,0xE8,0xCC,0xCC,0xCC,0xCC,
	0xFF,0x0D,0xDD,0xDD,0xDD,0xDD,
	0xB9,0x00,0x00,0x00,0x00,0x8B,0x46,0x40,0x8D,0x34,0x08,
	0x90,0x90,0x90,0x90,0x90,
	0x5F,0x5E,0x59,0x5B,0x5A,0x58,
	0x90,0x90,0x90,0x90,0x90, //store routine
	0x90,0x90,0x90,0x90,0x90, 0x90,0x90,0x90,0x90,0x90,
	0x90,0x90,0x90,0x90,0x90, //ipb's 0 and 1
	
	//force random idx (currently disabled)
	// 0x50,0x52,
	// 0xA1, 0x00,0x00,0x00,0x00,
	// 0x8B,0x90, 0x00,0x00,0x00,0x00,
	// 0x89,0x15, 0x00,0x00,0x00,0x00,
	// 0x5A,0x58
	0x50,
	0xA1,0x00,0x00,0x00,0x00,
	0x40,
	0x3D,0x58,0x02,0x00,0x00,
	0x75,0x02,
	0x31,0xC0,
	0x89,0x05,0x00,0x00,0x00,0x00,
	0x58
}; 
//rA (+8)  => current_game_ptr
//rB (+14) => allow_input_cycle_ptr
//rC (+22) => diff(InputCycle)
//rD (+28) => allow_input_cycle_ptr


///////////////////////////
//INPUT CYCLE LOCK PATCH //
///////////////////////////



//+0
//#define INPUT_CYCLE_OVERRIDE_SIZE 5
//BYTE input_cycle_find[] = {0x83,0xFF,0xFF,0xD9,0xFF,0xFF,0xD9,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x33,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x8D,0xFF,0xFF,0x69};
//std::string input_cycle_mask = "x..x..x......x......x..x";

//BYTE input_cycle_oper[] = {0xA1,0xAA,0xAA,0xAA,0xAA, 0x83,0xF8,0x00, 0x75,0x09, 0xA1,0xBB,0xBB,0xBB,0xBB, 0x8B,0x40,0x58, 0xC3};
//rA (+1)  => allow_input_cycle_ptr
//rB (+11) => current_game_ptr

#define INPUT_CYCLE_OVERRIDE_SIZE 6
BYTE input_cycle_find[] = {0x81,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x80,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x74,0xFF,0xE8,0xFF,0xFF,0xFF,0xFF,0x80};
std::string input_cycle_mask = "x.........x......x.x....x";

BYTE input_cycle_oper[] = {0xA1,0xAA,0xAA,0xAA,0xAA, 0x83,0xF8,0x01, 0x74,0x09, 0xA1,0xBB,0xBB,0xBB,0xBB, 0x8B,0x40,0x58, 0xC3, 0xA1,0xCC,0xCC,0xCC,0xCC, 0x8B,0x40,0x40};
//rA (+1)  => allow_input_cycle_ptr
//rB (+11) => current_game_ptr
//rC (+20) => current_game_ptr


///////////
// FUNCTIONS TO NOP
///////////
//+0

BYTE zero_inputs_find[] = {0x83,0xCC,0xCC,0xBA,0xCC,0xCC,0xCC,0xCC,0x33,0xCC,0x8D,0xCC,0xCC,0xCC,0xCC,0xCC,0x88,0xCC,0xCC,0x88};
std::string zero_inputs_mask = "x..x....x.x.....x..x";

BYTE zero_inputs_repl[] = {0xC3};


///////
// Input Buffer reset routine
///////

BYTE irp_end_find[] = {0xA1,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0x8B,0xCC,0xCC,0x6A,0x00,0x6A,0x00,0x6A,0x00,0x6A,0x00,0xCC,0xFF,0xCC,0x6A,0x01};
std::string irp_end_mask = "x....x.x..xxxxxxxx.x.xx";

//TODO set byte_12465EE to 0 when cancelling, move dwi_insert_find to before byte_12465EE check in WinMain
// basically, find a way to make the system delay work.

#define IB_PREV_ALLOC 8
#define IB_RESET_ROUTINE_ALLOC 64
#define IB_STORE_ROUTINE_ALLOC 64
BYTE ib_store_routine_base[] = {0x90,0x90,0x90,0x90,0x90, 0x90,0x90,0x90,0x90,0x90,0x90,
								0x90,0x90,0x90,0x90,0x90, 0x90,0x90,0x90,0x90,0x90,0x90};
BYTE ib_store_routine_chnk[] = {0xA1,0x00,0x00,0x00,0x00, 0x89,0x05,0x00,0x00,0x00,0x00};

BYTE ib_reset_routine_base[] = {0x90,0x90,0x90,0x90,0x90, 0x90,0x90,0x90,0x90,0x90,0x90,
								0x90,0x90,0x90,0x90,0x90, 0x90,0x90,0x90,0x90,0x90,0x90, 
								0x90,0x90,0x90,0x90,0x90}; //reset addrs, then jmp to main end

BYTE ib_reset_routine_chnk[] = {0xA1,0x00,0x00,0x00,0x00, 0x89,0x05,0x00,0x00,0x00,0x00};


/////////////
// Input Push Position Caching (for more reliable data related to the position of input push buffers)
/////////////
BYTE ib_cache_routine_base[] = {0x90,0x90,0x90,0x90,0x90, 0x90,0x90,0x90,0x90,0x90,0x90,
								0x90,0x90,0x90,0x90,0x90, 0x90,0x90,0x90,0x90,0x90,0x90};
BYTE ib_cache_routine_chnk[] = {0xA1,0x00,0x00,0x00,0x00, 0x89,0x05,0x00,0x00,0x00,0x00};

//////////////
// REMOVE FRAME MULTIPLIER
/////////////

//+0
BYTE ib_frame_mult_find[] = {0x83,0xCC,0xCC,0xCC,0x8D,0xCC,0xCC,0xCC,0xCC,0xFF,0xCC,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0x8B};
std::string ib_frame_mult_mask = "x...x....x.....x...x";
BYTE ib_frame_mult_override[] = {0xC3};

//+2
BYTE ib_frame_val_find[] = {0xDD,0xCC,0xCC,0xCC,0xCC,0xCC,0xDF,0xCC,0xCC,0xCC,0xCC,0xCC,0xDE,0xCC,0xDD,0xCC,0xCC,0xCC,0xCC,0xCC,0x83,0xCC,0xCC,0xC3};
std::string ib_frame_val_mask = "x.....x.....x.x.....x..x";
double ib_frame_val_force = 0.0167;

//+2
BYTE ib_frame_arb1_find[] = {0xDF,0xCC,0xCC,0xCC,0xCC,0xCC,0xDE,0xCC,0xDD,0xCC,0xCC,0xCC,0xCC,0xCC,0x93,0xCC,0xCC,0xC3};
std::string ib_frame_arb1_mask = "x.....x.x.....x..x";
BYTE ib_frame_arb1_force[] = {0x00,0x00,0x00,0xE0,0x7E,0xFD,0x77,0x40};

//+2
BYTE ib_frame_arb2_find[] = {0x89,0xCC,0xCC,0xCC,0xCC,0xCC,0x89,0xCC,0xCC,0xCC,0xCC,0xCC,0xDC,0xCC,0xCC,0xD9,0xCC,0xDD};
std::string ib_frame_arb2_mask = "x.....x.....x..x.x";
BYTE ib_frame_arb2_force[] = {0xA2,0x03,0x00,0x00};

//+2
BYTE ib_frame_arb3_find[] = {0x89,0xCC,0xCC,0xCC,0xCC,0xCC,0xDC,0xCC,0xCC,0xD9,0xCC,0xDD,0xCC,0xCC,0xCC,0xCC,0xCC,0xDF};
std::string ib_frame_arb3_mask = "x.....x..x.x.....x";
BYTE ib_frame_arb3_force[] = {0x59,0x6D,0xC2,0xF9};

////////////////////
// NO MOUSE PATCH //
////////////////////

BYTE ib_mouse_find[] = {0xE8,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0xCC,0xCC,0xE8,0xCC,0xCC,0xCC,0xCC,0xA1};
std::string ib_mouse_mask = "x....x.....x...x....x";

BYTE ib_mouse_override[] = {0x90, 0x90, 0x90, 0x90, 0x90};

////////////////////

InputReceivePatch::~InputReceivePatch() {
	//OPT theoretical memory leak on ~IRP if offset scan routine fails mid-way
	if(is_valid) {
		delete[] orig_jmpbuild_override;
		delete[] orig_main_override;
		delete[] orig_input_cycle;
		delete[] zero_inputs_orig;
		delete[] orig_frame_mult;
		delete[] orig_mouse;
	}
}

InputReceivePatch::InputReceivePatch(std::shared_ptr<DerandomizePatch> dp, std::shared_ptr<GameHooks> gh) : 
	Patch(dp->spel), 

	spel(dp->spel), 
	local_buffer_pos(0),
	input_buffer_position_ptr(0),
	input_buffer_ptr(0),
	dp(dp), 
	gh(gh), 

	orig_jmpbuild_override(nullptr),

	dwi(0),
	dwi_insert(0),
	dwi_exec_space(0),
	orig_main_override(nullptr),

	allow_input_cycle_ptr(0),
	input_cycle_fn(0),
	input_cycle_exec_space(0),
	orig_input_cycle(nullptr),

	zero_inputs_orig(nullptr),
	zero_inputs_addr(0),

	prev_ibufpos_addr(0),
	ibufpos_reset_routine(0),
	ibufpos_store_routine(0),

	bufpos_cache_vars(0),
	bufpos_routine(0),

	frame_mult_addr(0),
	frame_mult_func(0),
	orig_frame_mult(nullptr),
	frame_mult_fl_arb1(0),
	frame_mult_in_arb2(0),
	frame_mult_in_arb3(0),

	orig_mouse(nullptr),
	mouse_addr(0),

	is_valid(true)
{
	ipb[0] = nullptr;
	ipb[1] = nullptr;

	exec_space = spel->allocate(EXEC_SPACE_ALLOC_SIZE, true);
	if(!exec_space) 
	{
		DBG_EXPR(std::cout << "[InputReceivePatch] Failed to remote allocate exec_space" << std::endl);
		is_valid = false;
		return;
	}
	DBG_EXPR(std::cout << "Allocated InputReceivePatch exec_space at " << std::setbase(16) << exec_space << std::endl);



	//allocate input buffer + 4 bytes for buffer position (position /not/ used by frozlunky-side, manages its own buffer pos)
	input_buffer_position_ptr = spel->allocate(8);
	allow_input_cycle_ptr = spel->allocate(8);
	input_buffer_ptr = spel->allocate(INPUT_BUFFER_REMOTE_SIZE);

	if(!input_buffer_position_ptr || !allow_input_cycle_ptr || !input_buffer_ptr) {
		DBG_EXPR(std::cout << "[InputReceivePatch] Failed to allocate input buffer space" << std::endl);
		is_valid = false;
		return;
	}

	
	DBG_EXPR (
		std::cout << "[InputReceivePatch] Allocated remote position index at " << std::setbase(16) << input_buffer_position_ptr << std::endl;
		std::cout << "[InputReceivePatch] Allocated remote input buffer at " << std::setbase(16) << input_buffer_ptr << std::endl;
		std::cout << "[InputReceivePatch] Allocated input cycle lock at " << std::setbase(16) << allow_input_cycle_ptr << std::endl
	);



	//find recv jmp override
	{
		Address over = spel->find_mem(recv_patch_overwrite_find, recv_patch_overwrite_mask);
		if(!over) {
			DBG_EXPR(std::cout << "[InputReceivePatch] Failed to find jmp build overwrite address" << std::endl);
			is_valid = false;
			return;
		}
		DBG_EXPR(std::cout << "[InputReceivePatch] Determined receive patch address: " << std::setbase(16) << over << std::endl);

		overwrite_addr = over;

		orig_jmpbuild_override = new BYTE[RECV_PATCH_OVERWRITE_SIZE];
		spel->read_mem(overwrite_addr, orig_jmpbuild_override, RECV_PATCH_OVERWRITE_SIZE);
	}


	//allocate dwi exec
	{
		dwi_exec_space = spel->allocate(EXEC_SPACE_ALLOC_SIZE, true);
		if(!dwi_exec_space) {
			DBG_EXPR(std::cout << "[InputReceivePatch] Failed to allocate DWI exec space" << std::endl);
			is_valid = false;
			return;
		}
		DBG_EXPR(std::cout << "[InputReceivePatch] Allocated DWI exec space at " <<  dwi_exec_space << std::endl);
	}

	//find dwi
	{
		dwi = spel->find_mem(dwi_find, dwi_mask);
		if(!dwi) {
			DBG_EXPR(std::cout << "[InputReceivePatch] Failed to find DWI." << std::endl);
			is_valid = false;
			return;
		}
		DBG_EXPR(std::cout << "[InputReceivePatch] Found DWI at " << dwi << std::endl);
	}

	//find dwi insert addr + backup.
	{
		dwi_insert = spel->find_mem(dwi_insert_find, dwi_insert_mask);
		if(!dwi_insert) {
			DBG_EXPR(std::cout << "[InputReceivePatch] Failed to find DWI exec offset." << std::endl);
			is_valid = false;
			return;
		}
		dwi_insert += 0;
		DBG_EXPR(std::cout << "[InputReceivePatch] Inserting DWI execution at " << dwi_insert << std::endl);

		orig_main_override = new BYTE[DWI_OVERRIDE_SIZE];
		spel->read_mem(dwi_insert, orig_main_override, DWI_OVERRIDE_SIZE);
	}

	
	//allocate input cycle exec space
	{
		input_cycle_exec_space = spel->allocate(EXEC_SPACE_ALLOC_SIZE, true);
		if(!input_cycle_exec_space) {
			DBG_EXPR(std::cout << "[InputReceivePatch] Failed to allocate input_cycle_exec_space" << std::endl);
			is_valid = false;
			return;
		}
		DBG_EXPR(std::cout << "[InputReceivePatch] input_cycle_exec_space => " << input_cycle_exec_space << std::endl);
	}

	//find input cycle fn
	{
		input_cycle_fn = spel->find_mem(input_cycle_find, input_cycle_mask);
		if(!input_cycle_fn) {
			DBG_EXPR(std::cout << "[InputReceivePatch] Failed to find InputCycle function." << std::endl);
			is_valid = false;
			return;
		}
		DBG_EXPR(std::cout << "[InputReceivePatch] Found InputCycle function at " << input_cycle_fn << std::endl);
	
		orig_input_cycle = new BYTE[INPUT_CYCLE_OVERRIDE_SIZE];
		spel->read_mem(input_cycle_fn, orig_input_cycle, INPUT_CYCLE_OVERRIDE_SIZE);
	}


	//ZeroInputs patch-out
	{
		zero_inputs_addr = spel->find_mem(zero_inputs_find, zero_inputs_mask);
		if(!zero_inputs_addr) {
			DBG_EXPR(std::cout << "[InputReceivePatch] Failed to find ZeroInputs " << std::endl);
			is_valid = false;
			return;
		}
		DBG_EXPR(std::cout << "[InputReceivePatch] ZeroInputs at " << zero_inputs_addr << std::endl);

		zero_inputs_orig = new BYTE[sizeof(zero_inputs_repl)];
		spel->read_mem(zero_inputs_addr, zero_inputs_orig, sizeof(zero_inputs_repl));
	}


	//allocate input buffer position reset (for when either IRP's cancel the frame)
	{
		prev_ibufpos_addr = spel->allocate(IB_PREV_ALLOC);
		ibufpos_reset_routine = spel->allocate(IB_RESET_ROUTINE_ALLOC, true);
		ibufpos_store_routine = spel->allocate(IB_STORE_ROUTINE_ALLOC, true);
		if(!ibufpos_reset_routine || !prev_ibufpos_addr || !ibufpos_store_routine) {
			DBG_EXPR(std::cout << "[InputReceivePatch] Failed to allocate input buffer position reset memory." << std::endl);
			is_valid = false;
			return;
		}
		DBG_EXPR (
			std::cout << "[InputReceivePatch] prev_ibufpos_addr = " << prev_ibufpos_addr << std::endl;
			std::cout << "[InputReceivePatch] ibufpos_reset_routine = " << ibufpos_reset_routine << std::endl;
			std::cout << "[InputReceivePatch] ibufpos_store_routine = " << ibufpos_store_routine << std::endl
		);
	}

	//find main loop end
	{
		main_loop_end = spel->find_mem(irp_end_find, irp_end_mask);
		if(!main_loop_end) {
			DBG_EXPR(std::cout << "[InputReceivePatch] Failed to find main_loop_end" << std::endl);
			is_valid = false;
			return;
		}
		std::cout << "[InputReceivePatch] main_loop_end = " << main_loop_end << std::endl;
	}


	//allocate cache
	{
		bufpos_cache_vars = spel->allocate(8);
		bufpos_routine = spel->allocate(256, true);
		if(!bufpos_cache_vars || !bufpos_routine) {
			DBG_EXPR(std::cout << "[InputReceivePatch] bufpos alloc fail." << std::endl);
			is_valid = false;
			return;
		}
		DBG_EXPR(std::cout << "[InputReceivePatch] bufpos alloc at " << bufpos_cache_vars << ", routine = " << bufpos_routine << std::endl);
	}


	//find multiplier removal
	{
		Address container = spel->find_mem(ib_frame_val_find, ib_frame_val_mask);
		frame_mult_func = spel->find_mem(ib_frame_mult_find, ib_frame_mult_mask);
		frame_mult_in_arb2 = spel->find_mem(ib_frame_arb2_find, ib_frame_arb2_mask);
		frame_mult_in_arb3 = spel->find_mem(ib_frame_arb3_find, ib_frame_arb3_mask);

		if(!container 
		|| !frame_mult_func 
		|| !frame_mult_in_arb2 
		|| !frame_mult_in_arb3) 
		{
			DBG_EXPR(std::cout << "[InputReceivePatch] Failed to find multiplier removal info." << std::endl);
			is_valid = false;
			return;
		}
		
		orig_frame_mult = new BYTE[sizeof(ib_frame_mult_override)];
		spel->read_mem(frame_mult_func, orig_frame_mult, sizeof(ib_frame_mult_override));

		container += 2;
		frame_mult_in_arb2 += 2;
		frame_mult_in_arb3 += 2;
		
		spel->read_mem(container, &frame_mult_addr, sizeof(Address));
		spel->read_mem(container + 14, &frame_mult_fl_arb1, sizeof(Address));
		spel->read_mem(frame_mult_in_arb2, &frame_mult_in_arb2, sizeof(Address));
		spel->read_mem(frame_mult_in_arb3, &frame_mult_in_arb3, sizeof(Address));


		DBG_EXPR(std::cout << "[InputReceivePatch] frame_mult_addr = " << frame_mult_addr << ", frame_mult_func = " << frame_mult_func << std::endl);
		DBG_EXPR(std::cout << "[InputReceivePatch] frame_mult_fl_arb1 = " << frame_mult_fl_arb1 << std::endl);
		DBG_EXPR(std::cout << "[InputReceivePatch] frame_mult_in_arb2 = " << frame_mult_in_arb2 << std::endl);
		DBG_EXPR(std::cout << "[InputReceivePatch] frame_mult_in_arb3 = " << frame_mult_in_arb3 << std::endl);
	}

	//find mouse
	{
		mouse_addr = spel->find_mem(ib_mouse_find, ib_mouse_mask);
		if(!mouse_addr) {
			DBG_EXPR(std::cout << "[InputReceivePatch] Failed to find mouse_addr" << std::endl);
			is_valid = false;
			return;
		}
		orig_mouse = new BYTE[sizeof(ib_mouse_override)];
		spel->read_mem(mouse_addr, orig_mouse, sizeof(ib_mouse_override));
	}
}

bool InputReceivePatch::valid() {
	return is_valid;
}

bool InputReceivePatch::perform_irp_patch() 
{
	//zero remote irp state
	{
		unsigned zerov = 0;
		BYTE* zero = new BYTE[INPUT_BUFFER_REMOTE_SIZE];
		spel->write_mem(input_buffer_position_ptr, &zerov, sizeof(unsigned));
		spel->write_mem(allow_input_cycle_ptr, &zerov, sizeof(unsigned));
		spel->write_mem(input_buffer_ptr, zero, sizeof(zero));
		delete[] zero;
	}

	//write opcodes
	{
		spel->write_mem(exec_space, recv_patch_opcodes, sizeof(recv_patch_opcodes));
		 
		Address end_jne_diff = jne_diff(exec_space+9, exec_space+269);
		uint32_t ibs_div_ifs = INPUT_BUFFER_REMOTE_SIZE / INPUT_FRAME_REMOTE_SIZE;
		uint32_t input_frame_size = INPUT_FRAME_REMOTE_SIZE;

		//OPT non-static game (load game instance from spelunky-side?)
		Address game = 0;
		spel->read_mem(dp->game_ptr(), &game, sizeof(Address));

		Address run_switch = game + gh->run_switch_offs();
		Address counter_idx = dp->arb_alloc_counter_address();
		int max_player_val = 3;

		spel->write_mem(exec_space+5, &max_player_val, sizeof(int)); //r1
		spel->write_mem(exec_space+11, &end_jne_diff, sizeof(Address)); //r2
		spel->write_mem(exec_space+19, &input_buffer_position_ptr, sizeof(Address)); //r3
		spel->write_mem(exec_space+24, &ibs_div_ifs, sizeof(uint32_t)); //r4
		spel->write_mem(exec_space+34, &input_frame_size, sizeof(uint32_t)); //r5
		spel->write_mem(exec_space+41, &input_buffer_ptr, sizeof(Address)); //r6
		spel->write_mem(exec_space+54, &run_switch, sizeof(Address)); //r7
		spel->write_mem(exec_space+242, &counter_idx, sizeof(Address)); //r8
		spel->write_mem(exec_space+258, &input_buffer_position_ptr, sizeof(Address)); //r9
		spel->write_mem(exec_space+265, &input_buffer_position_ptr, sizeof(Address)); //rA
	}

	//create jump (we're not jumping here anymore from overwrite_addr, we're jumping from persistent_dwi_patch)
	//spel->jmp_build(overwrite_addr, RECV_PATCH_OVERWRITE_SIZE, exec_space, sizeof(recv_patch_opcodes));
	return true;
}

//TODO define a second slot for input push so inputs can be pushed for local player too
void InputReceivePatch::set_input_push(int slot, std::shared_ptr<InputPushBuilder> pb) {
	ipb[slot] = pb;
}

bool InputReceivePatch::perform_persistent_dwi_patch() 
{
	Address cgame_ptr = dp->game_ptr();
	Address dwi_diff = jmp_diff(dwi_exec_space+0x15, dwi);

	spel->write_mem(dwi_exec_space, dwi_insert_oper, sizeof(dwi_insert_oper));

	spel->write_mem(dwi_exec_space+8, &cgame_ptr, sizeof(Address));
	spel->write_mem(dwi_exec_space+14, &allow_input_cycle_ptr, sizeof(Address));
	spel->write_mem(dwi_exec_space+22, &dwi_diff, sizeof(Address));
	spel->write_mem(dwi_exec_space+28, &allow_input_cycle_ptr, sizeof(Address));

	spel->jmp_build(dwi_exec_space+43, 5, exec_space, sizeof(recv_patch_opcodes));
	
	///////////////////////
	//input push builders//
	///////////////////////

	spel->write_mem(ibufpos_store_routine, ib_store_routine_base, sizeof(ib_store_routine_base));
	
	//construct store + reset routines
	{
		spel->write_mem(ibufpos_reset_routine, ib_reset_routine_base, sizeof(ib_reset_routine_base));
		BYTE jmp_to_mainend[5] = {0xE9, 0xAA,0xAA,0xAA,0xAA};
		signed mainend_diff = jmp_diff(ibufpos_reset_routine + 22, main_loop_end);
		std::memcpy(jmp_to_mainend+1, &mainend_diff, sizeof(signed));
		spel->write_mem(ibufpos_reset_routine + 22, jmp_to_mainend, sizeof(jmp_to_mainend));

		//store routine
		{
			int offs = 0;
			int mem_offs = 0;
			for(std::shared_ptr<InputPushBuilder> pb : ipb) {
				if(pb) {
					spel->write_mem(ibufpos_store_routine+offs, ib_store_routine_chnk, sizeof(ib_store_routine_chnk));

					Address ibuf_pos = pb->ibuf_position_ptr();
					spel->write_mem(ibufpos_store_routine+offs+1, &ibuf_pos, sizeof(Address));
				
					Address store_pos = prev_ibufpos_addr + mem_offs;
					spel->write_mem(ibufpos_store_routine+offs+7, &store_pos, sizeof(Address));
				}
				offs += sizeof(ib_store_routine_chnk);
				mem_offs += 4;
			}
		}

		spel->jmp_build(dwi_exec_space+54, 5, ibufpos_store_routine, sizeof(ib_store_routine_base));
		
		//reset routine
		{
			int offs = 0;
			int mem_offs = 0;
			for(std::shared_ptr<InputPushBuilder> pb : ipb) {
				if(pb) {
					spel->write_mem(ibufpos_reset_routine+offs, ib_reset_routine_chnk, sizeof(ib_reset_routine_chnk));

					Address store_pos = prev_ibufpos_addr + mem_offs;
					spel->write_mem(ibufpos_reset_routine+offs+1, &store_pos, sizeof(Address));
					
					Address ibuf_pos = pb->ibuf_position_ptr();
					spel->write_mem(ibufpos_reset_routine+offs+7, &ibuf_pos, sizeof(Address));
				}
				offs += sizeof(ib_reset_routine_chnk);
				mem_offs += 4;
			}
		}
	}

	//TODO fix offs for second. ?
	int offs = 59;
	for(std::shared_ptr<InputPushBuilder> pb : ipb) {
		if(pb && pb->valid()) {
			pb->perform();
			spel->jmp_build(dwi_exec_space+offs, 5, pb->get_subroutine(), pb->get_subroutine_size());
			pb->set_cancel_routine(ibufpos_reset_routine);

			//write rate updater
			if(pb->get_pid() == 0) {
				pb->write_rate_updater(this->frame_mult_addr);
			}
		}
		offs += 5;
	}

	//bufpos cache routine
	{
		spel->write_mem(bufpos_routine, ib_cache_routine_base, sizeof(ib_cache_routine_base));

		int offs = 0;
		int mem_offs = 0;
		for(std::shared_ptr<InputPushBuilder> pb : ipb) {
			if(pb) {
				pb->set_cache_pos(bufpos_cache_vars+mem_offs);

				spel->write_mem(bufpos_routine+offs, ib_cache_routine_chnk, sizeof(ib_cache_routine_chnk));

				Address store_pos = prev_ibufpos_addr + mem_offs;
				spel->write_mem(bufpos_routine+offs+1, &store_pos, sizeof(Address));
					
				Address cache_pos = pb->get_cache_pos();
				spel->write_mem(bufpos_routine+offs+7, &cache_pos, sizeof(Address));
			}
			offs += sizeof(ib_cache_routine_chnk);
			mem_offs += 4;
		}

		spel->jmp_build(dwi_exec_space+69, 5, bufpos_routine, sizeof(ib_cache_routine_base));
	}

	///////////////////////
	// COUNTER INC / MOD //
	///////////////////////

	Address counter_addr = dp->arb_alloc_counter_address();
	spel->write_mem(dwi_exec_space+0x4B+1, &counter_addr, sizeof(Address));
	spel->write_mem(dwi_exec_space+0x5A+2, &counter_addr, sizeof(Address));
	
	////////////////////////
	// FORCE RANDOM PATCH //
	////////////////////////
	//{
	//	Address game_ptr = dp->game_ptr();
	//	unsigned level_offs = dp->current_level_offset();
	//	Address random_idx_addr = dp->random_idx_address();
	//
	//	spel->write_mem(dwi_exec_space+77, &game_ptr, sizeof(Address));
	//	spel->write_mem(dwi_exec_space+83, &level_offs, sizeof(unsigned));
	//	spel->write_mem(dwi_exec_space+89, &random_idx_addr, sizeof(Address));
	//}

	spel->jmp_build(dwi_insert, DWI_OVERRIDE_SIZE, dwi_exec_space, sizeof(dwi_insert_oper));
	return true;
}

//this is technically DWI patch now.
bool InputReceivePatch::perform_input_cycle_patch()
{
	Address cgame_ptr = dp->game_ptr();

	spel->write_mem(input_cycle_exec_space, input_cycle_oper, sizeof(input_cycle_oper));
	spel->write_mem(input_cycle_exec_space+1, &allow_input_cycle_ptr, sizeof(Address));
	spel->write_mem(input_cycle_exec_space+11, &cgame_ptr, sizeof(Address));
	spel->write_mem(input_cycle_exec_space+20, &cgame_ptr, sizeof(Address));

	spel->jmp_build(input_cycle_fn, INPUT_CYCLE_OVERRIDE_SIZE, input_cycle_exec_space, sizeof(input_cycle_oper));


	//remove frame multiplier
	spel->write_mem(frame_mult_func, ib_frame_mult_override, sizeof(ib_frame_mult_override));
	spel->write_mem(frame_mult_addr, &ib_frame_val_force, sizeof(double));
	spel->write_mem(frame_mult_fl_arb1, ib_frame_arb1_force, sizeof(ib_frame_arb1_force));
	spel->write_mem(frame_mult_in_arb2, ib_frame_arb2_force, sizeof(ib_frame_arb2_force));
	spel->write_mem(frame_mult_in_arb3, ib_frame_arb3_force, sizeof(ib_frame_arb3_force));

	//remove mouse
	spel->write_mem(mouse_addr, ib_mouse_override, sizeof(ib_mouse_override));
	return true;
}

bool InputReceivePatch::_perform() {
	bool suc = this->perform_irp_patch() && this->perform_persistent_dwi_patch() && this->perform_input_cycle_patch();
	return suc;
}

void InputReceivePatch::pull_inputs(std::vector<InputFrame>& out) {
	unsigned remote_buffer_pos = 0;
	spel->read_mem(input_buffer_position_ptr, &remote_buffer_pos, sizeof(unsigned));

	while(local_buffer_pos < remote_buffer_pos) 
	{
		unsigned real = local_buffer_pos % (INPUT_BUFFER_REMOTE_SIZE / INPUT_FRAME_REMOTE_SIZE);

		InputFrame frame = {};
		spel->read_mem(input_buffer_ptr + real*INPUT_FRAME_REMOTE_SIZE, &frame, INPUT_FRAME_REMOTE_SIZE);
		if(!frame.ready) {
			break;
		}
		
		out.push_back(frame);
		local_buffer_pos++;
	}
}

bool InputReceivePatch::_undo() {
	spel->write_mem(overwrite_addr, orig_jmpbuild_override, RECV_PATCH_OVERWRITE_SIZE);
	spel->write_mem(dwi_insert, orig_main_override, DWI_OVERRIDE_SIZE);
	spel->write_mem(input_cycle_fn, orig_input_cycle, INPUT_CYCLE_OVERRIDE_SIZE);
	spel->write_mem(zero_inputs_addr, zero_inputs_orig, sizeof(zero_inputs_repl));
	spel->write_mem(frame_mult_func, orig_frame_mult, sizeof(ib_frame_mult_override));
	spel->write_mem(mouse_addr, orig_mouse, sizeof(ib_mouse_override));
	return true;
}