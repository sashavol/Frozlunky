#pragma once

#include <memory>
#include <thread>
#include <chrono>

#include "spelunky.h"
#include "derandom.h"
#include "game_hooks.h"
#include "seeder.h"
#include "netplay_connection.h"
#include "async_input_grab.h"
#include "input_recv_patch.h"
#include "antipause_patch.h"
#include "save_manager.h"

#define INPUT_BUFFER_MAX_DEFAULT 8

enum NetplaySessionCloseEvent {
	NS_UNKNOWN, NS_DESYNC, NS_CORRUPT_DATA, NS_REQUESTED, NS_LOST_CONNECTION
};

struct InputFrame_Packet {
	InputFrame frame;
	unsigned order_id;
};

class NetplaySession {
private:
	typedef std::chrono::high_resolution_clock clock;

	struct NetplaySession_Controller {
		bool shutdown;

		NetplaySession_Controller::NetplaySession_Controller() : shutdown(false) {}
	};
	
private:
	bool closed;

	int pid;

	bool character_synced;

	std::shared_ptr<InputReceivePatch> irp;
	std::shared_ptr<NetplayConnection> conn;
	std::shared_ptr<AsyncInputGrab> aig;
	std::shared_ptr<Seeder> seeder;
	std::shared_ptr<InputPushBuilder> ipb_net;
	std::shared_ptr<InputPushBuilder> ipb_local;
	std::shared_ptr<GameHooks> gh;
	std::shared_ptr<AntipausePatch> antipause;
	std::shared_ptr<DerandomizePatch> dp;

	std::shared_ptr<NetplaySession_Controller> controller;
	std::function<void(int)> ping_info_cb;
	std::function<void(int)> buf_info_cb;

	std::shared_ptr<SaveManager> sm;

	std::thread worker_thread;

	std::vector<InputFrame> last_local_iframes;
	InputFrame last_net_iframe;

	//buffer for ordering packets as they come in
	std::vector<InputFrame_Packet> packet_buffer;

	unsigned input_counter;
	unsigned last_order_id;
	unsigned total_packets;
	int input_buffer_max;

	int no_push_counter;

	clock::time_point ping_out;
	clock::time_point last_frame_push;

	inline void push_input(InputFrame frame);

public:
	NetplaySession(int pid, 
		std::shared_ptr<InputPushBuilder> ipb_net, 
		std::shared_ptr<InputPushBuilder> ipb_local,
		std::shared_ptr<NetplayConnection> conn, 
		std::shared_ptr<InputReceivePatch> irp, 
		std::shared_ptr<Seeder> seeder,
		std::shared_ptr<GameHooks> hooks,
		std::shared_ptr<DerandomizePatch> dp);

	void ping_info(std::function<void(int)> cb);
	bool is_closed();
	void set_input_buffer_max(int val);
	void create_worker_thread();
	void close(NetplaySessionCloseEvent evt);
	int get_pid();
	void flush_buffers();
	void buf_change_cb(std::function<void(int)> cb);
};