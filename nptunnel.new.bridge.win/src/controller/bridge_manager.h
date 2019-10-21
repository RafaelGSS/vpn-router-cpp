#pragma once
#include <src/socket/udp/udp_socket.h>
#include <src/socket/tcp/tcp_socket.h>

class bridge_manager
{
private:
	udp_socket* _sock_udp;
	tcp_socket* _sock_tcp;

protected:
	void run_tcp();
	void run_udp();
public:
	void release_all();
	void release_tcp();
	void release_udp();

	void run();
	bridge_manager();
	~bridge_manager();
};

