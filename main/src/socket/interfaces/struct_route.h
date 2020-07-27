#pragma once
#include <stdint.h>
#include <string>

#pragma pack(1)
enum route_t : unsigned char {
	route_none,
	route_from_out,
	route_assigned,
	route_from_client,
	route_from_server,
	route_hop_from_client,
	route_hop_from_server
};

struct struct_route_header {
	uint32_t hash;
	route_t route_type;
	char auth_token[24];
	unsigned char total_route_index;
	unsigned char current_route_index;

	struct_route_header();
};


struct struct_ip_port_header {
	uint32_t ip;
	uint16_t port;
};

struct struct_ip_port_manager
	: public struct_ip_port_header
{
	struct_ip_port_manager(uint32_t _ip, uint16_t _port);
	struct_ip_port_manager(std::pair<uint32_t, uint16_t>& _in);
	struct_ip_port_manager();
	bool is_null();
};


struct struct_route_manager 
	: public struct_route_header
{
	struct_ip_port_manager route_table[1];

	bool is_route_valid(uint32_t len);

	bool has_loop_route(); // Understand it more later
	bool has_next_route();
	// Check if hash is valid
	bool validate_route_hash();
	
	void update_route_index();
	// Update hash of security
	void update_route_hash();

	uint32_t get_this_route_size();
	struct_ip_port_manager get_current_route();
	struct_ip_port_manager get_next_route();

	void set_auth_token(const std::string& token);

	struct_route_manager(
		std::string _auth_token, 
		uint8_t _current_route_index, 
		uint8_t _route_count, 
		route_t _route_type,
		struct_ip_port_manager* routes
	);
};


#pragma pack()

