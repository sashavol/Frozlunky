#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <vector>
#include <map>
#include <string>
#include <thread>

#include <enet/enet.h>

#define CHANNEL_INTERNAL 0
#define CHANNEL_HEARTBEAT 1

enum NetplayDisconnectEvent {
	ND_TIMEOUT, ND_PEER_DISCONNECT, ND_UNKNOWN
};

std::string NDFriendlyString(NetplayDisconnectEvent nd);

//wrapper for enet packets, intended to be used with managed memory.
struct NetplayPacket {
	ENetPacket* _packet;

	void* data;
	size_t len;

public:
	NetplayPacket(ENetPacket* p);
	NetplayPacket(void* data, size_t len);
	~NetplayPacket();

	friend class NetplayConnection;
};

//handles NetplayConnection::Host (async kill switch currently)
class HostHandle {
private:
	bool kill;

public:
	HostHandle() : kill(false) {}
	
	bool killed() {
		return kill;
	}

	void set_killed(bool val) {
		kill = val;
	}
};

//nothing to handle in NetplayConnection::Connect!
class ConnectHandle {
public:
	ConnectHandle() {}
};

//Async two-way connection model (one peer only)
class NetplayConnection {
public:
	typedef std::function<void(std::shared_ptr<NetplayConnection>)> conn_cb;
	typedef std::function<bool(std::shared_ptr<NetplayPacket>)> data_fn;
	typedef std::function<void(NetplayDisconnectEvent)> disconnect_fn;

private:
	struct _internal_packet {
		BYTE* packet_mem;
		size_t len;

		_internal_packet(int c, void* m, size_t l);
		void destroy();
	};

private:
	bool destroyed;

	//general-purpose callback mutex
	std::recursive_mutex cbm;

	//worker thread
	std::mutex worker_sig_mutex;
	bool worker_kill_sig;
	bool worker_active;
	HANDLE worker;

	//internal enet conn
	std::mutex enet_mut;
	std::mutex queue_mut;
	std::vector<_internal_packet> send_packet_queue;
	ENetHost* host;
	ENetAddress* addr;
	ENetPeer* peer;

	//callbacks on data retrieval from peer
	std::map<int, std::vector<data_fn>> data_cbs;

	//peer disconnect callbacks
	std::vector<disconnect_fn> disconnect_cbs;

	void destroy_connection(NetplayDisconnectEvent reason = ND_UNKNOWN);

	static DWORD __stdcall _internalthread(void* ptr);

public:
	NetplayConnection(ENetHost* h, ENetAddress* a, ENetPeer* p);
	~NetplayConnection();

	//Calls the passed data function on new peer data (parameter is packet)
	void on_data(int channel, data_fn fn);

	//Calls the passed function on disconnect
	void on_disconnect(disconnect_fn fn);

	//send a packet to the peer
	void send(int channel, void* data, size_t len);

	//starts listening for data
	void start();
	void close();

	//behave as host and wait for connection
	static std::shared_ptr<HostHandle> Host(int port, conn_cb hcb);

	//behave as client and connect to specified target
	static std::shared_ptr<ConnectHandle> Connect(const std::string& host, int port, conn_cb cb);

	static bool Initialize();
	static bool Deinitialize();
};