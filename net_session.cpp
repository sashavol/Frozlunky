#include "debug.h"
#include "net_session.h"
#include "seed_synchronizer.h"

#include <iostream>
#include <sstream>

namespace MP {
	NetSession::NetSession(std::shared_ptr<Connection> connection, int player,
		std::shared_ptr<Seeder> seeder) : 
		connection(connection), player(player) 
	{
		add_synch(std::make_shared<SeedSynchronizer>(seeder));
		connection->recv_callback(create_recv_callback());
	}

	//Recv callback will be called every time new information is received from connected, wires to correct synchronizer
	Connection::recv_cb NetSession::create_recv_callback() {
		return [this](void* buffer, size_t len) {
			self_mutex.lock();

			char* data = (char*)buffer;
			NetSession::synch_data_targ targ_synch = NetSession::DecodeSynchRequest(data);
			unsigned target_synchronizer = targ_synch.first;
			
			auto synchronizer_iter = synchronizers.find(target_synchronizer);
			if(synchronizer_iter != synchronizers.end()) {
				synchronizer_iter->second->synchronize(targ_synch.second);
			}
#ifdef DEBUG_MODE
			else {
				std::cout << "[NetSession] Warning: Unknown synchronizer type: " << target_synchronizer << std::endl;
			}
#endif

			self_mutex.unlock();
		};
	}

	//synch_targ<4>|data
	void NetSession::EncodeSynchRequest(data_text_type& out, synch_data_targ synch_targ) {
		unsigned targ = synch_targ.first;
		std::copy(&targ, &targ+sizeof(unsigned), std::back_inserter(out));
		synch_targ.second->dump(out);
	}

	NetSession::synch_data_targ NetSession::DecodeSynchRequest(char* data) {
		unsigned targ = *(unsigned*)data;
		data += sizeof(unsigned);
		
		std::shared_ptr<SynchData> synch_data = SynchData::Parse(data);
		return synch_data_targ(targ, synch_data);
	}

	void NetSession::tick() {
		self_mutex.lock();
		if(player == PLAYER_SENDER) {
			std::vector<char> encoded;
			
			for(std::pair<unsigned, std::shared_ptr<Synchronizer>> synch : synchronizers) {
				std::shared_ptr<SynchData> data = synch.second->retrieve();
				EncodeSynchRequest(encoded, synch_data_targ(synch.first, data));

				connection->send(encoded.data(), encoded.size(), [](bool result) {
#ifdef DEBUG_MODE
					if(!result) {
						std::cout << "[NetSession] Warning: Failed to send synchronize request." << std::endl;
					}
#endif
				});
			
				encoded.clear();
			}
		}
		self_mutex.unlock();
	}
}
