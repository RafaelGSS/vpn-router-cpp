#include "bridge_manager.h"

#include <src/settings/file_managment/file_manager.h>

void bridge_manager::run_udp()
{
	auto config = file_manager_bridge_base::singleton();
	//wpp::settings::priority::elevate_priority(config->get_core_afinnity_num());

	uint16_t udp_port			= config->get_udp_port();
	std::string server_ip		= config->get_address();
	std::string server_bind_ip	= config->get_bind_address();

	if (server_bind_ip.empty())
		server_bind_ip = server_ip;

	if (!udp_port) {
		LOGE("\nERROR does not have a specified UDP port");
		return;
	}
	
	_sock_udp = new udp_socket(udp_port, server_ip, server_bind_ip);
	int cores = config->get_core_afinnity_num();
	if(cores > 2)
		cores = (cores % 2 == 0) ? cores / 2 : (cores / 2) + 1; /* +1 To udp */
	
	_sock_udp->run(cores);
}

void bridge_manager::release_all()
{
	release_tcp();
	release_udp();
}

void bridge_manager::release_tcp()
{
	if (_sock_tcp != nullptr) {
		delete _sock_tcp;
		_sock_tcp = nullptr;
	}
}

void bridge_manager::release_udp()
{
	if (_sock_udp != nullptr) {
		delete _sock_udp;
		_sock_udp = nullptr;
	}
}

void bridge_manager::run_tcp()
{

	auto config = file_manager_bridge_base::singleton();
	wpp::settings::priority::elevate_priority(config->get_core_afinnity_num());

	uint16_t tcp_port			= config->get_tcp_port();
	std::string server_ip		= config->get_address();
	std::string server_bind_ip	= config->get_bind_address();

	if (server_bind_ip.empty())
		server_bind_ip = "0.0.0.0";

	if (!tcp_port) {
		LOGE("\nERROR does not have a specified UDP port");
		return;
	}

	
	int cores = config->get_core_afinnity_num();
	if (cores > 2)
		cores /= 2;

	_sock_tcp = new tcp_socket(tcp_port, server_bind_ip);
	_sock_tcp->run(cores);
}

void bridge_manager::run()
{
	boost::thread(&bridge_manager::run_udp, this).detach();
	boost::thread(&bridge_manager::run_tcp, this).detach();
}

bridge_manager::bridge_manager() :
	_sock_tcp(nullptr), _sock_udp(nullptr)
{
}

bridge_manager::~bridge_manager()
{
	release_all();
}
