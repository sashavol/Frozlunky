#include "input_push_builder.h"
#include "debug.h"

#define IPB_EXEC_ALLOC 1500
#define IPB_VARS_ALLOC 32
#define IPB_INPUT_BUFFER_ALLOC (INPUT_FRAME_REMOTE_SIZE*8192)

//!TODO allocate counter (incremented each frame) and store (its value % 600) to random_idx if it's pid == 0

BYTE ipb_subroutine[] = {0x56,0x57,0x50,0x52,0x53,0x51,0x8B,0x35,0x78,0x00,0x2F,0x09,0xEB,0x3A,0x90,0x74,0x37,0x83,0xFE,0x17,0x74,0x32,0x83,0xFE,0x1A,0x74,0x2D,0x83,0xFE,0x16,0x74,0x28,0x83,0xFE,0x0B,0x74,0x23,0x83,0xFE,0x02,0x74,0x1E,0x83,0xFE,0x03,0x74,0x19,0x83,0xFE,0x11,0x74,0x14,0x83,0xFE,0x1E,0x74,0x0F,0x83,0xFE,0x12,0x74,0x0A,0x83,0xFE,0x1F,0x74,0x05,0xE9,0xC1,0x01,0x00,0x00,0xA1,0x60,0x00,0x2F,0x09,0xBF,0x98,0x00,0x00,0x00,0x01,0xC7,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0xA1,0x00,0x00,0x13,0x20,0x8B,0x15,0x04,0x00,0x16,0x06,0x39,0xD0,0x7C,0x0C,0xC6,0x05,0x00,0x00,0x16,0x06,0x01,0xE9,0x6B,0x01,0x00,0x00,0xA1,0x00,0x00,0x13,0x20,0x31,0xD2,0xBB,0x00,0x00,0x01,0x00,0xF7,0xF3,0x8B,0xC2,0xBB,0x2B,0x00,0x00,0x00,0xF7,0xE3,0x05,0x04,0x00,0x13,0x20,0x8B,0xF0,0x8B,0x46,0x1D,0xEB,0x40,0x90,0x74,0x3D,0x83,0xF8,0x17,0x74,0x38,0x83,0xF8,0x1A,0x74,0x33,0x83,0xF8,0x16,0x74,0x2E,0x83,0xF8,0x0B,0x74,0x29,0x83,0xF8,0x02,0x74,0x24,0x83,0xF8,0x03,0x74,0x1F,0x83,0xF8,0x11,0x74,0x1A,0x83,0xF8,0x1E,0x74,0x15,0x83,0xF8,0x12,0x74,0x10,0x83,0xF8,0x1F,0x74,0x0B,0xFF,0x05,0x00,0x00,0x13,0x20,0xE9,0x5B,0xFF,0xFF,0xFF,0x8B,0x0D,0x60,0x00,0x2F,0x09,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
	0xC6,0x05,0xCC,0xCC,0xCC,0xCC,0x00,0x90,0x8A,0x46,0x01,0x88,0x07,0x8A,0x46,0x02,0x88,0x47,0x01,0x8A,0x46,0x03,0x88,0x47,0x02,0x8A,0x46,0x04,0x88,0x47,0x03,0x8A,0x46,0x05,0x88,0x47,0x05,0x8A,0x46,0x06,0x88,0x47,0x0D,0x8B,0x46,0x07,0x89,0x47,0x1C,0x8B,0x46,0x0B,0x89,0x47,0x20,0x8A,0x46,0x0F,0x88,0x47,0x04,0x8A,0x46,0x10,0x88,0x47,0x06,0x8A,0x46,0x11,0x88,0x47,0x07,0x8A,0x46,0x12,0x88,0x47,0x08,0x8A,0x46,0x13,0x88,0x47,0x09,0x8A,0x46,0x14,0x88,0x47,0x0A,0x8A,0x46,0x15,0x88,0x47,0x0B,0x8A,0x46,0x16,0x88,0x47,0x0E,0x8A,0x46,0x17,0x88,0x47,0x0F,0x8A,0x46,0x18,0x88,0x47,0x10,0x8A,0x46,0x19,0x88,0x47,0x11,0x8A,0x46,0x1A,0x88,0x47,0x12,0x8A,0x46,0x1B,0x88,0x47,0x13,0x8A,0x46,0x1C,0x88,0x47,0x14,0x90,0x90,0x90,0x90,0x90,0x90,0x8A,0x46,0x24,0x88,0x47,0x0C,0x8A,0x46,0x25,0x88,0x47,0x16,0x8A,0x46,0x26,0x88,0x47,0x17,0x90,0x90,0x90,0x90,0x90,0x90,0x8A,0x46,0x28,0x88,0x47,0x19,0x8A,0x46,0x29,0x88,0x47,0x1A,0x8A,0x46,0x2A,0x88,0x47,0x1B,0x8A,0x46,0x1C,0x88,0x47,0x14,0x90,0x90,0x90,0x90,0x90,0x90,0x8A,0x46,0x24,0x88,0x47,0x0C,0x8A,0x46,0x25,0x88,0x47,0x16,0x8A,0x46,0x26,0x88,0x47,0x17,0x90,0x90,0x90,0x90,0x90,0x90,0x8A,0x46,0x28,0x88,0x47,0x19,0x8A,0x46,0x29,0x88,0x47,0x1A,0xFF,0x05,0x00,0x00,0x00,0x00,0xEB,0x0B,0x5E,0x5F,0x58,0x5A,0x5B,0x59,0xE9,0x37,0x6B,0x0A,0xFA,0x5E,0x5F,0x58,0x5A,0x5B,0x59};

//inserted at ipb_subroutine + 0xFF
BYTE force_idx_subroutine[] = {0x8B,0x46,0x1D,0x89,0x05,0xAA,0xAA,0xAA,0xAA,0x89,0x05,0xAA,0xAA,0xAA,0xAA};

//TODO implement inserting these (for pid == 1 only)
//     this will require a pipe spanning input_recv_patch -> this

//pid == 0 only 
//lea edx, [frame_mult_addr]
//inserted at +0x199
BYTE force_mult1_subroutine[] = {0x8D,0x15,0xAA,0xAA,0xAA,0xAA};

//pid == 0 only
//mov eax, [esi+2B]
//mov [edx], eax
//inserted at +0x1B1
BYTE force_mult2_subroutine[] = {0x8B,0x46,0x2B,0x89,0x02};

//pid == 0 only
//mov eax, [esi+2F]
//mov [edx+4], eax
//inserted at +0x1CF
BYTE force_mult3_subroutine[] = {0x8B,0x46,0x2F,0x89,0x42,0x04};



//+0
BYTE ipb_end_find[] = {0xA1,0xCC,0xCC,0xCC,0xCC,0x8B,0xCC,0x8B,0xCC,0xCC,0x6A,0x00,0x6A,0x00,0x6A,0x00,0x6A,0x00,0xCC,0xFF,0xCC,0x6A,0x01};
std::string ipb_end_mask = "x....x.x..xxxxxxxx.x.xx";

//+14
BYTE ipb_run_switch_offset_find[] = {0x83,0xFF,0xFF,0x75,0xFF,0x8B,0xFF,0xFF,0xFF,0xFF,0xFF,0x8D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x33,0xFF,0x39,0xFF,0x0F};
std::string ipb_run_switch_offset_mask = "x..x.x.....x......x.x.x";

//+21
BYTE ipb_antihold_find[] = {0xCC,0x8B,0xCC,0xCC,0xE8,0xCC,0xCC,0xCC,0xCC,0x83,0xCC,0xCC,0x77,0xCC,0xFF,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xC6,0xCC,0xCC,0xCC,0xCC,0xC3};
std::string ipb_antihold_mask = ".x..x....x..x.x......x....x";

BYTE ipb_antihold_override[] = {0x90,0x90,0x90,0x90,0x5E,0xC3,0x90,0x90,0x90,0x90,0x5E,0xC3,0xC6,0x46,0x08,0x01,0x5E,0xC3,
	0x90,0x90,0x90,0x90,0x5E,0xC3,0x90,0x90,0x90,0x90,0x5E,0xC3,0x90,0x90,0x90,0x90,0x5E,0xC3,0x90,0x90,0x90,0x90,0x5E,0xC3,
	0x90,0x90,0x90,0x90,0x5E,0xC3,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x5E,0xC3,0x90,0x90,0x90,0x90,0x5E,0xC3,0xC3}; //{0x5E, 0xC3, 0x90, 0x90,0x90,0x90,0x90};

//+0
BYTE ipb_antihold2_find[] = {0x88,0xCC,0x06,0x0F,0xCC,0xCC,0xCC,0x88,0xCC,0x07,0x0F,0xCC,0xCC,0xCC,0x88,0xCC,0x08};
std::string ipb_antihold2_mask = "x.xx...x.xx...x.x";

BYTE ipb_antihold2_override[] = {0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
	0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
	0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
	0x90,0x90,0x90,0x90,0x33,0xD2,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,
	0x90,0x90,0x90};

InputPushBuilder::~InputPushBuilder() {
	if(antihold_orig) {
		delete[] antihold_orig;
		antihold_orig = nullptr;
	}

	if(antihold2_orig) {
		delete[] antihold2_orig;
		antihold2_orig = nullptr;
	}
}

void InputPushBuilder::flush() {
	int flpos = input_buffer_position - 1;
	spel->write_mem(input_buffer_position_ptr, &flpos, sizeof(int));
	spel->write_mem(froz_buffer_position_ptr, &flpos, sizeof(int));
}

InputPushBuilder::InputPushBuilder(int p, std::shared_ptr<DerandomizePatch> dp, std::shared_ptr<GameHooks> gh) 
	: spel(dp->spel), 
	dp(dp), 
	gh(gh), 
	pid(p),
	input_buffer_position(0), 
	main_loop_end(0), 
	subroutine(0), 
	input_wait_buffer_size(0),

	antihold_addr(0),
	antihold_orig(nullptr),
	antihold2_addr(0),
	antihold2_orig(nullptr),

	cache_pos(0),
	
	is_valid(true)
{
	input_wait_mode_ptr = spel->allocate(IPB_VARS_ALLOC);
	if(!input_wait_mode_ptr) {
		DBG_EXPR(std::cout << "[InputPushBuilder] Failed to allocate vars memory." << std::endl);
		is_valid = false;
		return;
	}
	froz_buffer_position_ptr = input_wait_mode_ptr + 4;
	DBG_EXPR(std::cout << "[InputPushBuilder] Allocated vars memory at " << input_wait_mode_ptr << std::endl);

	input_buffer_position_ptr = spel->allocate(8); //4 ifrs padding
	input_buffer_ptr = spel->allocate(IPB_INPUT_BUFFER_ALLOC);
	if(!input_buffer_position_ptr || !input_buffer_ptr) {
		DBG_EXPR(std::cout << "[InputPushBuilder] Failed to allocate input buffer." << std::endl);
		is_valid = false;
		return;
	}

	DBG_EXPR(std::cout << "[InputBufferPosition] Allocated input buffer position at " << input_buffer_position_ptr << std::endl);
	DBG_EXPR(std::cout << "[InputBufferPosition] Allocated input buffer at " << input_buffer_ptr << std::endl);


	subroutine = spel->allocate(IPB_EXEC_ALLOC, true);
	if(!subroutine) {
		DBG_EXPR(std::cout << "[InputPushBuilder] Failed to allocate subroutine memory." << std::endl);
		is_valid = false;
		return;
	}
	DBG_EXPR(std::cout << "[InputPushBuilder] Allocated subroutine memory at " << subroutine << std::endl);


	Address main_end = spel->find_mem(ipb_end_find, ipb_end_mask);
	if(!main_end) {
		DBG_EXPR(std::cout << "[InputPushBuilder] Failed to find main routine exit." << std::endl);
		is_valid = false;
		return;
	}
	main_loop_end = main_end;
	DBG_EXPR(std::cout << "[InputPushBuilder] Found main loop end at " << main_end << std::endl);

	
	antihold_addr = spel->find_mem(ipb_antihold_find, ipb_antihold_mask);
	if(!antihold_addr) {
		DBG_EXPR(std::cout << "[InputPushBuilder] Failed to find antihold" << std::endl);
		is_valid = false;
		return;
	}
	antihold_addr += 21;
	DBG_EXPR(std::cout << "[InputPushBuilder] Found antihold at " << antihold_addr << std::endl);
	antihold_orig = new BYTE[sizeof(ipb_antihold_override)];
	spel->read_mem(antihold_addr, antihold_orig, sizeof(ipb_antihold_override));
	

	antihold2_addr = spel->find_mem(ipb_antihold2_find, ipb_antihold2_mask);
	if(!antihold2_addr) {
		DBG_EXPR(std::cout << "[InputPushBuilder] Failed to find antihold2" << std::endl);
		is_valid = false;
		return;
	}
	DBG_EXPR(std::cout << "[InputPushBuilder] Found antihold2 at " << antihold2_addr << std::endl);
	antihold2_orig = new BYTE[sizeof(ipb_antihold2_override)];
	spel->read_mem(antihold2_addr, antihold2_orig, sizeof(ipb_antihold2_override));
}

bool InputPushBuilder::valid() {
	return is_valid;
}

Address InputPushBuilder::get_subroutine() {
	return subroutine;
}

size_t InputPushBuilder::get_subroutine_size() {
	return sizeof(ipb_subroutine);
}

void InputPushBuilder::undo() {
	spel->write_mem(antihold_addr, antihold_orig, sizeof(ipb_antihold_override));
	spel->write_mem(antihold2_addr, antihold2_orig, sizeof(ipb_antihold2_override));
}

void InputPushBuilder::perform() {
	spel->write_mem(subroutine, ipb_subroutine, sizeof(ipb_subroutine));

	Address game_ptrptr = dp->game_ptr();
	Address game_ptr = 0;
	spel->read_mem(game_ptrptr, &game_ptr, sizeof(Address));

	Address cgp_pl_so = game_ptr + gh->game_state_offs();
	spel->write_mem(subroutine + 0x06 + 2, &cgp_pl_so, sizeof(Address));

	Address cgp_pl_0x40 = game_ptr + 0x40;
	spel->write_mem(subroutine + 0x48 + 1, &cgp_pl_0x40, sizeof(Address));

	unsigned ctrl_size = gh->ctrl_size();
	uint32_t ctrlsize_mul_pidaffect = ctrl_size * pid;
	spel->write_mem(subroutine + 0x4D + 1, &ctrlsize_mul_pidaffect, sizeof(uint32_t));
	//spel->write_mem(subroutine + 0x54 + 1, &input_wait_mode_ptr, sizeof(Address));
	//spel->write_mem(subroutine + 0x5E + 1, &froz_buffer_position_ptr, sizeof(Address));
	//spel->write_mem(subroutine + 0x63 + 2, &input_buffer_position_ptr, sizeof(Address));
	//spel->write_mem(subroutine + 0x69 + 2, &input_wait_buffer_size, sizeof(BYTE));
	//spel->write_mem(subroutine + 0x74 + 2, &input_wait_mode_ptr, sizeof(Address));
	spel->write_mem(subroutine + 0x78 + 1 + 3, &input_buffer_position_ptr, sizeof(Address));
	spel->write_mem(subroutine + 0x7D + 2 + 3, &froz_buffer_position_ptr, sizeof(Address));
	spel->write_mem(subroutine + 0x87 + 2 + 3, &input_wait_mode_ptr, sizeof(Address));
	spel->write_mem(subroutine + 0x93 + 1 + 3, &input_buffer_position_ptr, sizeof(Address));

	uint32_t idx_count = IPB_INPUT_BUFFER_ALLOC / INPUT_FRAME_REMOTE_SIZE;
	spel->write_mem(subroutine + 0x9A + 1 + 3, &idx_count, sizeof(uint32_t));

	uint32_t frame_size = INPUT_FRAME_REMOTE_SIZE;
	spel->write_mem(subroutine + 0xA3 + 1 + 3, &frame_size, sizeof(uint32_t));
	spel->write_mem(subroutine + 0xAA + 1 + 3, &input_buffer_ptr, sizeof(uint32_t));
	spel->write_mem(subroutine + 0xEB + 2 + 3, &input_buffer_position_ptr, sizeof(input_buffer_position_ptr));
	spel->write_mem(subroutine + 0xF6 + 2 + 3, &cgp_pl_so, sizeof(Address));
	//spel->write_mem(subroutine + 0x100 + 2 + 3, &input_buffer_position_ptr, sizeof(Address));

	//force random idx if data is from pid 0
	if(pid == 0) {
		DBG_EXPR(std::cout << "[InputPushBuilder] (pid == 0), writing force routine." << std::endl);

		Address force = dp->arb_alloc_force_address();
		Address random_idx = dp->random_idx_address();
		spel->write_mem(subroutine + 0xFF, force_idx_subroutine, sizeof(force_idx_subroutine));
		spel->write_mem(subroutine + 0x102 + 2, &force, sizeof(Address));
		spel->write_mem(subroutine + 0x108 + 2, &random_idx, sizeof(Address));
	}
	
	unsigned run_switch_offset = gh->run_switch_offs();
	uint32_t cgp_4pidaf_rso = game_ptr + 4*pid + run_switch_offset;
	spel->write_mem(subroutine + 0x10E + 2, &cgp_4pidaf_rso, sizeof(uint32_t));

	//
	spel->write_mem(subroutine + 0x1F9 + 2, &input_buffer_position_ptr, sizeof(Address));

	Address loopend_diff = jmp_diff(subroutine + 0x204 + 3, main_loop_end);
	spel->write_mem(subroutine + 0x204 + 1 + 3, &loopend_diff, sizeof(Address));


	//remove antihold
	spel->write_mem(antihold_addr, ipb_antihold_override, sizeof(ipb_antihold_override));
	spel->write_mem(antihold2_addr, ipb_antihold2_override, sizeof(ipb_antihold2_override));
}

int InputPushBuilder::buffer_size() {
	unsigned froz_pos = input_buffer_position;
	
	unsigned spel_pos;
	if(!cache_pos) {
		spel->read_mem(input_buffer_position_ptr, &spel_pos, sizeof(unsigned));
	}
	else {
		spel->read_mem(cache_pos, &spel_pos, sizeof(unsigned));
	}

	return froz_pos - spel_pos;
}

void InputPushBuilder::set_cache_pos(Address c) {
	cache_pos = c;
}

Address InputPushBuilder::get_cache_pos() {
	return cache_pos;
}

int InputPushBuilder::get_pid() {
	return this->pid;
}

void InputPushBuilder::push_input(const InputFrame& frame) {
	Address buffer_target = input_buffer_ptr + ((input_buffer_position*INPUT_FRAME_REMOTE_SIZE) % (IPB_INPUT_BUFFER_ALLOC));
	spel->write_mem(buffer_target, &frame, INPUT_FRAME_REMOTE_SIZE);
	input_buffer_position++;
	spel->write_mem(froz_buffer_position_ptr, &input_buffer_position, sizeof(int));
}

void InputPushBuilder::set_cancel_routine(Address jmpout) {
	main_loop_end = jmpout;

	//rewrite in case it's already performed
	Address loopend_diff = jmp_diff(subroutine + 0x204 + 3, main_loop_end);
	spel->write_mem(subroutine + 0x204 + 1 + 3, &loopend_diff, sizeof(Address));
}

Address InputPushBuilder::ibuf_position_ptr() {
	return input_buffer_position_ptr;
}

void InputPushBuilder::destroy() {
	if(subroutine) {
		spel->free(subroutine);
		subroutine = 0;
	}
}

void InputPushBuilder::write_rate_updater(Address frame_mult_addr) {
	spel->write_mem(subroutine + 0x199, force_mult1_subroutine, sizeof(force_mult1_subroutine));
	spel->write_mem(subroutine + 0x1B1, force_mult2_subroutine, sizeof(force_mult2_subroutine));
	spel->write_mem(subroutine + 0x1CF, force_mult3_subroutine, sizeof(force_mult3_subroutine));
	spel->write_mem(subroutine + 0x199 + 2, &frame_mult_addr, sizeof(Address));
}