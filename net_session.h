#pragma once

#include <string>
#include <memory>
#include <vector>
#include <mutex>

#include "synch.h"
#include "connection.h"
#include "seeder.h"

#define PLAYER_SENDER 1

namespace MP {
	class NetSession {
	private:
		typedef std::vector<char> data_text_type;
		typedef std::pair<unsigned, std::shared_ptr<SynchData>> synch_data_targ;

		std::shared_ptr<Connection> connection;
		std::map<unsigned, std::shared_ptr<Synchronizer>> synchronizers;
		std::mutex self_mutex;
		int player;

		Connection::recv_cb create_recv_callback();
		void send_synch();

		static void EncodeSynchRequest(data_text_type& encode, synch_data_targ synch);
		static synch_data_targ DecodeSynchRequest(char* data);

		template <class _Synchronizer>
		void add_synch(std::shared_ptr<_Synchronizer> synch) {
			unsigned class_hash = typeid(_Synchronizer).hash_code();
			synchronizers[class_hash] = synch;
		}

	public:
		NetSession(std::shared_ptr<Connection> connection, int player, std::shared_ptr<Seeder> seeder);

		void tick();
	};
}