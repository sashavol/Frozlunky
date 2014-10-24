#pragma once

#include <string>
#include <functional>
#include <memory>

namespace MP {
	class Connection {
	public:
		typedef std::function<void(bool)> send_result_cb;
		typedef std::function<void(void*, size_t)> recv_cb;
		typedef std::function<void(std::shared_ptr<Connection>)> conn_cb;

	private:
		std::string ip;
		recv_cb receive_callback;
		
	public:
		Connection(const std::string& ip);

		void send(void* data, size_t len, send_result_cb callback);
		void recv_callback(recv_cb callback);

		static void Establish(std::string ip, int port, conn_cb connection);
		static void Listen(int port, conn_cb connection);
	};
}