#pragma once
#include <stdint.h>
#include <string>

struct alias_info {
	uint32_t ipv4_address;
	uint16_t tcp_port;
	uint16_t udp_port;

	std::string ipv4_address_string();
	void set_ipv4_address(std::string str_ip);

	bool is_null_tcp();
	bool is_null_udp();
	bool is_null_address();

	alias_info();
	alias_info(uint32_t _ipv4_address, uint16_t _tcp_port, uint16_t _udp_port);
	alias_info(std::string ip, std::string _tcp_port, std::string _udp_port);
};