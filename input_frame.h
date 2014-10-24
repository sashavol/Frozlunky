#pragma once

#include <string>
#include <sstream>

#define INPUT_FRAME_REMOTE_SIZE 62

#define IFRAME_AH_JUMP 0x10
#define IFRAME_AH_WHIP 0x11
#define IFRAME_AH_ROPE 0x12
#define IFRAME_AH_DOOR 0x13

struct InputFrame {
	uint8_t run_toggle;
	
	uint8_t jump;
	uint8_t bomb;
	uint8_t whip;
	uint8_t rope;
	uint8_t enter_door;
	uint8_t run_key;
	
	uint8_t _x_axis[4];
	uint8_t _y_axis[4];
	
	uint8_t unk[11];
	uint8_t pause;
	uint8_t unk2[2];
	
	uint8_t _game_state[4];
	uint8_t ready;
	uint8_t pid;

	uint8_t unk3[8];
	uint8_t game_rate[sizeof(double)];

	BYTE __pad[16];
	

	double rate() {
		return *(double*)game_rate;
	}

	void rate(double val) {
		*(double*)game_rate = val;
	}

	int game_state() const {
		return *(int*)_game_state;
	}

	int x_axis() const {
		return *(int*)_x_axis;
	}

	int y_axis() const {
		return *(int*)_y_axis;
	}

	std::string to_string() const {
		std::stringstream ss;

		ss << "{ " << ":" << (int)pid << " " << (int)game_state() << " ";
#define exists_print(fi) if(##fi) std::cout << #fi << " ";
		exists_print(jump);
		exists_print(bomb);
		exists_print(whip);
		exists_print(rope);
		exists_print(enter_door);
		exists_print(run_key);
		exists_print(x_axis());
		exists_print(y_axis());
		exists_print(pause);
#undef exists_print
		std::cout << "}" << std::endl;

		return ss.str();
	}
};