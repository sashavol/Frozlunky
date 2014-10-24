#include "netplay_session.h"
#include "force_controller_attrs.h"
#include "custom_hud.h"
#include "gui.h"

#define CHANNEL_INPUTS 2
#define CHANNEL_SEEDS 3
#define CHANNEL_KILL 4
#define CHANNEL_STATE 5
#define CHANNEL_BUFFER_MAX 6
#define CHANNEL_PINGPONG 7
#define CHANNEL_INIT 8
#define CHANNEL_SETTINGS 9

#define PING_INTERVAL 333

void NetplaySession::push_input(InputFrame frame) {
	if(ipb_net) {
		ipb_net->push_input(frame);
		last_net_iframe = frame;
	}
}

void NetplaySession::create_worker_thread() {
	ping_out = clock::now();

	worker_thread = std::thread([=]() {
		while(true) {
			if(closed) {
				return;
			}

			clock::time_point time_now = clock::now();
			if(std::chrono::duration_cast<std::chrono::milliseconds>(time_now - ping_out).count() >= PING_INTERVAL) {
				ping_out = time_now;
				conn->send(CHANNEL_PINGPONG, (void*)"PING", 5);
			}

			//no longer necessary due to automated save loading
			/*
			if(gh->game_state() == STATE_CHARSELECT) {
				//set default characters
				gh->character_id(0, 0);
				gh->character_id(1, 3);
				//OPT, right now it is permanently set to 0 and 2, because with this method we can guarantee the characters are synced.
				//character_synced = true;
			}
			*/

			std::this_thread::sleep_for(std::chrono::milliseconds(14));
		}
	});

	conn->on_data(CHANNEL_PINGPONG, [=](std::shared_ptr<NetplayPacket> packet) {
		if(controller->shutdown) {
			return false;
		}

		std::string str((char*)packet->data);
		if(str == "PONG") {
			if(ping_info_cb) {
				ping_info_cb((int)std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - ping_out).count());
			}
		}
		else if(str == "PING") {
			conn->send(CHANNEL_PINGPONG, (void*)"PONG", 5);
		}
		else {
			DBG_EXPR(std::cout << "[NetplaySession] Received wrong ping size." << std::endl);
		}

		return true;
	});
}

void NetplaySession::ping_info(std::function<void(int)> cb) {
	ping_info_cb = cb;
}

int NetplaySession::get_pid() {
	return pid;
}

void NetplaySession::flush_buffers() {
	aig->query_flush_buffers();
	ipb_local->flush();
	ipb_net->flush();
}

#define LOCAL_IFRAME_HISTORY_SIZE 1

NetplaySession::NetplaySession(int pid, std::shared_ptr<InputPushBuilder> pb_net, 
										std::shared_ptr<InputPushBuilder> pb_local, 
										std::shared_ptr<NetplayConnection> c,
										std::shared_ptr<InputReceivePatch> i,
										std::shared_ptr<Seeder> s, 
										std::shared_ptr<GameHooks> g,
										std::shared_ptr<DerandomizePatch> d)
  : conn(c),
	irp(i),
	seeder(s),
	gh(g),
	input_counter(0),
	last_order_id(0),
	total_packets(0),
	input_buffer_max(INPUT_BUFFER_MAX_DEFAULT),
	closed(false),
	ipb_net(pb_net),
	ipb_local(pb_local),
	dp(d),
	character_synced(false),
	pid(pid),
	no_push_counter(0)
{
	{
		InputFrame empty = {};
		*((int*)empty._game_state) = -1;
		last_net_iframe = empty;
	}

	//force virtual player 2 to keyboard, 3 and 4 to none
	force_controller_type(gh, dp, 1, CONTROLLER_KEYBOARD);
	force_controller_type(gh, dp, 2, CONTROLLER_NONE);
	force_controller_type(gh, dp, 3, CONTROLLER_NONE);
	gh->ctrl_reset_x360(0);
	gh->ctrl_reset_x360(1);
	gh->damsel_type(3);
	gh->set_fullscreen_effects(FS_EFFECT_OFF);
	gh->set_dyn_shadows(false);
	gh->set_pro_hud(true);

	create_worker_thread();

	controller = std::make_shared<NetplaySession_Controller>();
	antipause = std::make_shared<AntipausePatch>(gh->spel);

	antipause->perform();

	conn->on_disconnect([=](NetplayDisconnectEvent) {
		close(NS_LOST_CONNECTION);
	});

	//flush any gathered inputs in IRP, they are unsafe and unnecessary.
	{
		std::vector<InputFrame> flush;
		irp->pull_inputs(flush);
	}

	aig = std::make_shared<AsyncInputGrab>(irp, [=](InputFrame frame) 
	{
		if(controller->shutdown) {
			return;
		}

		if(no_push_counter > 0) {
			no_push_counter--;
			return;
		}

		if(ipb_local->buffer_size() < input_buffer_max) 
		{
			BYTE* raw = (BYTE*)&frame;

			for(InputFrame last_local_iframe : last_local_iframes) {
				BYTE* raw_last = (BYTE*)&last_local_iframe;

				raw[0x10] = raw_last[0x1];
				raw[0x11] = raw_last[0x2];
				raw[0x12] = raw_last[0x3];
				raw[0x13] = raw_last[0x4];
				raw[0x14] = raw_last[0xF];
				raw[0x15] = raw_last[0x5];
				raw[0x16] = raw_last[0x24];
				raw[0x17] = raw_last[0x6];
				raw[0x1A] = raw_last[0x18];
				raw[0x1B] = raw_last[0x19];
			}

			if(last_local_iframes.size() == 0) {
				raw[0x10] = raw[0x11] = raw[0x12] = raw[0x13] = 
					raw[0x14] = raw[0x15] = raw[0x16] = raw[0x17] = 
						raw[0x1A] = raw[0x1B] = raw[0x1] = raw[0x2] =
							raw[0x3] = raw[0x4] = raw[0xF] = raw[0x5] = 
								raw[0x24] = raw[0x6] = raw[0x18] = raw[0x19] = 0;
			}
			
			//disable pausing if player 2, this caused the game to lock up before.
			if(pid == 1) {
				raw[0x1a] = 0;
				raw[0x18] = 0;
				raw[0x19] = 0;
				raw[0x1b] = 0;
			}

			std::cout << "[ ";
			for(int i = 0; i < sizeof(InputFrame); i++) {
				if(raw[i]) {
					std::cout << "raw[" << i << "] ";
				}
			}
			std::cout << " ]" << std::endl;

			InputFrame pushed = frame;
			BYTE* raw_p = (BYTE*)&pushed;

			if(ipb_local) {
				ipb_local->push_input(pushed);
			}
			
			last_local_iframes.push_back(frame);
			if(last_local_iframes.size() > LOCAL_IFRAME_HISTORY_SIZE) {
				last_local_iframes.erase(last_local_iframes.begin());
			}
			
			InputFrame_Packet p = {pushed, input_counter++};
			conn->send(CHANNEL_INPUTS, &p, sizeof(InputFrame_Packet));
		}
	});


	conn->on_data(CHANNEL_INPUTS, [=](std::shared_ptr<NetplayPacket> packet) 
	{
		if(controller->shutdown) {
			return false;
		}

		if(packet->len == sizeof(InputFrame_Packet)) 
		{
			InputFrame_Packet* ifp = reinterpret_cast<InputFrame_Packet*>(packet->data);
			InputFrame frame = ifp->frame;

			if(total_packets == 0 || ifp->order_id == last_order_id+1) 
			{
				//execute buffer + latest packet

				if(!packet_buffer.empty()) 
				{
					//sort the input packet buffer
					std::sort(packet_buffer.begin(), packet_buffer.end(), 
						[](const InputFrame_Packet& a, const InputFrame_Packet& b) {
							return a.order_id < b.order_id;
						}
					);

					for(InputFrame_Packet p : packet_buffer) {
						push_input(p.frame);
					}
					packet_buffer.clear();
				}

				//push last input
				push_input(ifp->frame);
				last_order_id = ifp->order_id;
			}
			else {
				packet_buffer.push_back(*ifp);
			}

			total_packets++;
		}
		else {
			//close(NS_CORRUPT_DATA);
			std::cout << "mismatched size" << std::endl;
		}

		return true;
	});


	//////////////////
	// SEED SYNC    //
	//////////////////

	//only player 1 may change the seed
	if(pid == 0) 
	{
		seeder->on_seed_change([=](const std::string& seed) 
		{
			if(controller->shutdown) {
				return false;
			}
		
			conn->send(CHANNEL_SEEDS, (void*)seed.c_str(), strlen(seed.c_str())+1);
			return true;
		});
		
		{
			std::string seed = seeder->get_seed();
			conn->send(CHANNEL_SEEDS, (void*)seed.c_str(), strlen(seed.c_str())+1);
		}
	}
	else 
	{
		conn->on_data(CHANNEL_SEEDS, [=](std::shared_ptr<NetplayPacket> packet) {
			if(controller->shutdown) {
				return false;
			}

			std::string seed = (char*)packet->data;
			seeder->seed(seed);
			return true;
		});
	}

	//////////////////
	// PLAYER KILL
	//////////////////

	conn->on_data(CHANNEL_KILL, [=](std::shared_ptr<NetplayPacket> packet) {
		if(controller->shutdown) {
			return false;
		}

		gh->set_health(0, 0);
		gh->set_health(1, 0);
		return true;
	});

	//////////////////
	// MAX BUFFER SYNC
	//////////////////

	if(pid == 1) 
	{
		conn->on_data(CHANNEL_BUFFER_MAX, [=](std::shared_ptr<NetplayPacket> packet) {
			if(controller->shutdown) {
				return false;
			}

			if(packet->len == sizeof(int)) {
				input_buffer_max = *(int*)packet->data;
				no_push_counter = input_buffer_max/2 + 1;

				if(buf_info_cb) {
					buf_info_cb(input_buffer_max);
				}

				DBG_EXPR(std::cout << "[NetplaySession] Set input_buffer_max to " << input_buffer_max << std::endl);
			}
			else {
				DBG_EXPR(std::cout << "[NetplaySession] Warning: Received incorrect size channel buffer packet on CHANNEL_BUFFER_MAX" << std::endl);
			}
			return true;
		});
	}
	else {
		conn->send(CHANNEL_BUFFER_MAX, &input_buffer_max, sizeof(int));
	}
}

//shuts down binded connection, undos all activated patches
void NetplaySession::close(NetplaySessionCloseEvent evt) 
{
	if(!closed) {
		closed = true;

		worker_thread.join();

		force_controller_type(gh, dp, 1, CONTROLLER_NONE);
		ipb_net->undo();
		ipb_local->undo();
		antipause->undo();
		controller->shutdown = true;
		aig.reset();
		irp->undo();
		exit(0); //TODO make netplay session actually disconnect properly rather than just close Frozlunky.
		//conn.reset(); //connection will close automatically, or is already closed.
	}
}

void NetplaySession::buf_change_cb(std::function<void(int)> cb) {
	this->buf_info_cb = cb;
}

void NetplaySession::set_input_buffer_max(int val) {
	if(!closed && pid == 0) {
		input_buffer_max = val;

		if(buf_info_cb) {
			buf_info_cb(input_buffer_max);
		}

		DBG_EXPR(std::cout << "[NetplaySession] Sending input buffer max of " << val << std::endl);
		conn->send(CHANNEL_BUFFER_MAX, &val, sizeof(int));
	}
}

bool NetplaySession::is_closed() {
	return closed;
}
