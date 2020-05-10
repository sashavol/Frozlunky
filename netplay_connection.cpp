#include "winheaders.h"
#include "netplay_connection.h"
#include "debug.h"

#include <time.h>
#include <iostream>

#define CONNECTION_TIMEOUT 5000
#define NO_EVENT_RECV_TIMEOUT 5000
#define TIME_HEARTBEAT 2000

////////////////
// MISC       //
////////////////

std::string NDFriendlyString(NetplayDisconnectEvent nd) {
	switch(nd) {
	case ND_TIMEOUT:
		return "The peer stopped responding.";
	case ND_PEER_DISCONNECT:
		return "The peer disconnected.";
	case ND_UNKNOWN:
		return "The connection was closed for an unknown reason.";
	}

	return "Unknown";
}

///////////////////////////
// PACKET WRAPPER        //
///////////////////////////
NetplayPacket::NetplayPacket(ENetPacket* p) : data(p->data), len(p->dataLength), _packet(p) {}

NetplayPacket::NetplayPacket(void* data, size_t len) : data(data), len(len) {
	_packet = enet_packet_create(data, len, ENET_PACKET_FLAG_RELIABLE);
}

NetplayPacket::~NetplayPacket() {
	if(_packet) {
		enet_packet_destroy(_packet);
    } else if (data) {
        delete[] data;
        data = nullptr;
    }
}

///////////////////////////
// INITIALIZING ENET     //
///////////////////////////

bool enet_init_success = false;

bool NetplayConnection::Initialize() {
	if(enet_initialize() != 0) {
		DBG_EXPR(std::cout << "[NetplayConnection] An error occurred while initializing Enet." << std::endl);
		enet_init_success = false;
		return false;
	}
	enet_init_success = true;
	return true;
}

bool NetplayConnection::Deinitialize() {
	if(enet_init_success) {
		enet_deinitialize();
	}

	return true;
}


///////////////////////////
// CONNECTION IMPL       //
///////////////////////////

NetplayConnection::NetplayConnection(ENetHost* h, ENetAddress* a, ENetPeer* p) :
	host(h),
	addr(a),
	peer(p),
	worker_kill_sig(false),
	worker_active(false),
    destroyed(false)
{}

void NetplayConnection::close() {
	this->destroy_connection();
}

DWORD __stdcall NetplayConnection::_internalthread(void* ptr) 
{
	NetplayConnection* t = reinterpret_cast<NetplayConnection*>(ptr);

	ENetEvent evt;
	int timeouts = 0;

	t->send(CHANNEL_INTERNAL, (void*)"READY", 6);
	
	auto flush_queue = [=]() {
		if(t->destroyed) {
			return;
		}

		t->queue_mut.lock();
		for(_internal_packet& p : t->send_packet_queue) {
			ENetPacket* packet = enet_packet_create(p.packet_mem, p.len, ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(t->peer, 0, packet);
		}
		t->send_packet_queue.clear();
		t->queue_mut.unlock();
	};

	//TODO time out when no events using NO_EVENT_RECV_TIMEOUT

	time_t time_recv;
	time_recv = time(0);
	
	time_t time_heartbeat;
	time_heartbeat = time(0);

	while(true) 
	{
		if(t->destroyed) {
			return 0;
		}

		flush_queue();

		t->enet_mut.lock();
		while(enet_host_service(t->host, &evt, 0) > 0)
		{
			t->worker_sig_mutex.lock();
			if(t->worker_kill_sig) {
				t->worker_active = false;
				return 0;
			}
			t->worker_sig_mutex.unlock();

			switch(evt.type) 
			{
			case ENET_EVENT_TYPE_RECEIVE:
				{
					time_recv = time(0);

					ENetPacket* p = evt.packet;
					
					int channel;
					std::memcpy(&channel, p->data, sizeof(int));

					std::shared_ptr<NetplayPacket> np = std::make_shared<NetplayPacket>(p);
					np->data = (BYTE*)np->data + sizeof(int);
					np->len -= sizeof(int);

					t->cbm.lock();
					auto it = t->data_cbs.find(channel);
					if(it != t->data_cbs.end()) {
						auto& sec = it->second;
						for(auto fi = sec.begin(); fi != sec.end();) {
							if(!((*fi)(np))) {
								fi = sec.erase(fi);
							}
							else {
								fi++;
							}
						}
					}
					t->cbm.unlock();

					flush_queue();
				}
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				{
					t->worker_sig_mutex.lock();
					t->worker_active = false;
					t->worker_sig_mutex.unlock();

					t->destroy_connection(ND_PEER_DISCONNECT);
				}
				return 0;
			}
		}

		time_t now = time(0);
		if(now - time_recv >= NO_EVENT_RECV_TIMEOUT) {
			t->destroy_connection(ND_TIMEOUT);
			return 0;
		}

		if(t->destroyed) {
			return 0;
		}

		t->enet_mut.unlock();

		t->worker_sig_mutex.lock();
		if(t->worker_kill_sig) {
			return 0;
		}
		t->worker_sig_mutex.unlock();

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}	
}

void NetplayConnection::start() 
{
	DWORD t;
	worker = CreateThread(NULL, 0, NetplayConnection::_internalthread, this, 0, &t);

	worker_sig_mutex.lock();
	worker_active = true;
	worker_sig_mutex.unlock();
}

std::mutex global_conn_destroy;

void NetplayConnection::destroy_connection(NetplayDisconnectEvent reason) 
{
	global_conn_destroy.lock();

	if(destroyed) {
		global_conn_destroy.unlock();
		return;
	}

	destroyed = true;

	for(disconnect_fn fn : disconnect_cbs) {
		fn(reason);
	}
	
	try {
		worker_sig_mutex.lock();
		if(worker_active) {
			//end worker
			worker_kill_sig = true;
			worker_sig_mutex.unlock();
			WaitForSingleObject(worker, INFINITE);
			worker_active = false;
		}
		else {
			worker_sig_mutex.unlock();
		}
	}
	catch(std::exception e) {
		std::cout << "[NetplayConnection] Warning: " << e.what() << std::endl;
	}

	if(peer) {
		enet_peer_disconnect(peer, 0);
		enet_peer_reset(peer);
	}

	if(host) {
		enet_host_destroy(host);
	}

	if(addr) {
		delete addr;
	}

	addr = nullptr;
	peer = nullptr;
	host = nullptr;

	global_conn_destroy.unlock();
}

NetplayConnection::_internal_packet::_internal_packet(int c, void* m, size_t l) 
	: len(4+l)
{
	packet_mem = new BYTE[l+sizeof(int)];
	std::memcpy(packet_mem, &c, sizeof(int));
	std::memcpy(packet_mem+sizeof(int), m, l);
}

void NetplayConnection::_internal_packet::destroy() {
	delete[] packet_mem;
}

void NetplayConnection::send(int c, void* data, size_t len) {
	queue_mut.lock();
	if(peer) {
		send_packet_queue.push_back(_internal_packet(c, data, len));
	}
	queue_mut.unlock();
}

NetplayConnection::~NetplayConnection() {
	destroy_connection();
}

void NetplayConnection::on_data(int channel, data_fn fn) 
{
	cbm.lock();

	auto it = data_cbs.find(channel);
	if(it != data_cbs.end()) 
	{
		data_cbs[channel].push_back(fn);
		cbm.unlock();
		return;
	}
	
	data_cbs[channel] = std::vector<data_fn>();
	data_cbs[channel].push_back(fn);

	cbm.unlock();
}

void NetplayConnection::on_disconnect(disconnect_fn fn) {
	cbm.lock();
	disconnect_cbs.push_back(fn);
	cbm.unlock();
}


//////////////////////////
// CONNECTION INIT IMPL //
//////////////////////////

struct HostCreate_Data {
	std::shared_ptr<HostHandle> handle;
	NetplayConnection::conn_cb hcb;
	int port;
};

DWORD __stdcall HostCreate_Thread(void* ptr) {
	HostCreate_Data* d = reinterpret_cast<HostCreate_Data*>(ptr);

	ENetAddress* address;
	ENetHost* server;

	address = new ENetAddress();
	address->host = ENET_HOST_ANY;
	address->port = d->port;
	
	server = enet_host_create(address, 1, 2, 0, 0);

	ENetPeer* peer;
	
	if(server == NULL) {
		DBG_EXPR(std::cout << "[NetplayConnection] Failed to create server." << std::endl);
		d->hcb(nullptr);

		delete d;
		return 0;
	}

	ENetEvent evt;
	while(true) 
	{
		while(enet_host_service(server, &evt, 1000)) {
			if(d->handle->killed()) {
				enet_host_destroy(server);
				d->hcb(nullptr);
				delete d;
				return 0;
			}

			std::cout << "Event: " << evt.type << std::endl;

			switch(evt.type) {
			case ENET_EVENT_TYPE_CONNECT:
				DBG_EXPR(std::cout << "[NetplayConnection] Connection from " << evt.peer->address.host << ":" << evt.peer->address.port << std::endl);
				peer = evt.peer;
				d->hcb(std::make_shared<NetplayConnection>(server, address, peer));

				delete d;
				return 0;
			case ENET_EVENT_TYPE_RECEIVE:
				DBG_EXPR(std::cout << "[NetplayConnection] Connection from " << evt.peer->address.host << ":" << evt.peer->address.port << std::endl);
				peer = evt.peer;
				d->hcb(std::make_shared<NetplayConnection>(server, address, peer));

				delete d;
				return 0;
			}
		}

		if(d->handle->killed()) {
			enet_host_destroy(server);
			d->hcb(nullptr);
			delete d;
			return 0;
		}
	}

	//we should never hit this
	enet_host_destroy(server);
	delete d;
	return 0;
}

//TODO implement handler that will allow cancelling hosting / connecting
std::shared_ptr<HostHandle> NetplayConnection::Host(int port, conn_cb hcb) 
{
	std::shared_ptr<HostHandle> handle = std::make_shared<HostHandle>();
	HostCreate_Data* d = new HostCreate_Data;
	d->handle = handle;
	d->port = port;
	d->hcb = hcb;

	DWORD t;
	CreateThread(NULL, 0, HostCreate_Thread, d, 0, &t);
	
	return handle;
}


//////////////

struct ConnectCreate_Data {
	std::shared_ptr<ConnectHandle> handle;
	NetplayConnection::conn_cb hcb;
	std::string host_str;
	int port;
};

DWORD __stdcall ConnectCreate_Thread(void* ptr) 
{
	ConnectCreate_Data* d = reinterpret_cast<ConnectCreate_Data*>(ptr);

	ENetHost* client;
	ENetPeer* peer;

	client = enet_host_create(NULL, 1, 255, 0, 0); //OPT not unlimited bandwidth
	if(client == NULL) {
		DBG_EXPR(std::cout << "[NetplayConnection] Failed to create client." << std::endl);
		d->hcb(nullptr);

		delete d;
		return 0;
	}

	ENetAddress* address = new ENetAddress();
	address->port = d->port;
	enet_address_set_host(address, d->host_str.c_str());

	peer = enet_host_connect(client, address, 0, 0);
	if(peer == NULL) {
		DBG_EXPR(std::cout << "[NetplayConnection] Failed to connect to peer." << std::endl);
		d->hcb(nullptr);

		delete d;
		return 0;
	}

	ENetEvent evt;
	if(enet_host_service(client, &evt, CONNECTION_TIMEOUT) > 0 && evt.type == ENET_EVENT_TYPE_CONNECT) {
		DBG_EXPR(std::cout << "[NetplayConnection] Connected to " << d->host_str << std::endl);
		auto conn = std::make_shared<NetplayConnection>(client, address, peer);
		d->hcb(conn);
	}
	else {
		DBG_EXPR(std::cout << "[NetplayConnection] Connection timed out." << std::endl);
		d->hcb(nullptr);
	}

	delete d;
	return 0;
}

std::shared_ptr<ConnectHandle> NetplayConnection::Connect(const std::string& host_str, int port, conn_cb hcb) 
{
	std::shared_ptr<ConnectHandle> handle = std::make_shared<ConnectHandle>();

	ConnectCreate_Data* d = new ConnectCreate_Data();
	d->host_str = host_str;
	d->port = port;
	d->hcb = hcb;
	d->handle = handle;

	DWORD t;
	CreateThread(NULL, 0, ConnectCreate_Thread, d, 0, &t);

	return handle;
}

