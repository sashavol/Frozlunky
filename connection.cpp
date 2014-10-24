#include "connection.h"

namespace MP {
	Connection::Connection(const std::string& ip) : ip(ip) {}


	void Connection::send(void* data, size_t len, send_result_cb callback) {
		
	}

	void Connection::recv_callback(recv_cb callback) {
		this->receive_callback = callback;
	}

	void Connection::Establish(std::string ip, int port, conn_cb connection) {
		std::shared_ptr<Connection> conn = std::make_shared<Connection>("127.0.0.1");
		connection(conn);
	}

	void Connection::Listen(int port, conn_cb connection) {
		std::shared_ptr<Connection> conn = std::make_shared<Connection>("127.0.0.1");
		connection(conn);
	}
}