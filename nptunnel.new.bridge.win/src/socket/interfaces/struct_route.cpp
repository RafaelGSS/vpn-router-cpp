#pragma once
#include <src/socket/interfaces/struct_route.h>
#include <src/socket/packet_manipulation/intercall_methods.h>

#include <wpp/net/ip/socket/defines.h>

#include <iostream>

bool struct_route_manager::is_route_valid(uint32_t len)
{

	// Check if packet size is less than struct
	if (len < sizeof(struct_route_manager))
		return false;

	// Check route out of bounds
	if (current_route_index >= total_route_index) {
		return false;
	}

	// Check packet is lowe than specifed route index count
	if (len < (sizeof(struct_route_manager) + sizeof(struct_ip_port_manager)* (total_route_index - 1))) {
		return false;
	}

	// I DONT UNDERSTAND IT YET
	if (has_loop_route()) {
		LOGD("here");
		//WPP_ACOUT << "\n fail to validade route table loop";
		return false;
	}

	// Check if actual hash is valid
	if (!validate_route_hash()) {
		LOGD("here hash");
		return false;
	}

	return true;
}

bool struct_route_manager::has_loop_route()
{
	for (uint32_t _i = 0; _i < total_route_index; _i++) {
		for (uint32_t _i2 = 0; _i2 < total_route_index; _i2++) {
			if (_i == _i2)
				continue;

			if (route_table[_i].ip == route_table[_i2].ip
				&& route_table[_i].port == route_table[_i2].port
				) {
				
				return true;
			}
		}
	}
	return false;
}

bool struct_route_manager::has_next_route()
{
	return ((current_route_index + 1) < total_route_index);
}

bool struct_route_manager::validate_route_hash()
{
	auto valid_hash = intercall_methods::default_generate_hash(((char*)this) + 4, get_this_route_size() - 4);
	if (hash == valid_hash)
		return true;
	return false;
}

void struct_route_manager::update_route_index()
{
	current_route_index++;
}

void struct_route_manager::update_route_hash()
{
	hash = intercall_methods::default_generate_hash(((char*)this) + 4, get_this_route_size() - 4);
}

uint32_t struct_route_manager::get_this_route_size()
{
	return sizeof(struct_route_manager) + (total_route_index - 1) * sizeof(struct_ip_port_manager);
}

struct_ip_port_manager struct_route_manager::get_current_route()
{
	return route_table[current_route_index];
}

struct_ip_port_manager struct_route_manager::get_next_route()
{
	return route_table[current_route_index + 1];
}

void struct_route_manager::set_auth_token(const std::string & token)
{
	if (token.size() != sizeof(auth_token)) {
		LOGD("auth token size invalid");
		return;
	}
	memcpy(auth_token, token.data(), sizeof(auth_token));
}

struct_route_manager::struct_route_manager(
	std::string _auth_token, 
	uint8_t _current_route_index, 
	uint8_t _route_count, 
	route_t _route_type,
	struct_ip_port_manager* routes
){
	set_auth_token(_auth_token);
	current_route_index = _current_route_index;
	total_route_index = _route_count;
	route_type = _route_type;

	memcpy(&(route_table)[0], routes, sizeof(struct_ip_port_manager)* _route_count);
}



// IPV4 STRUCT MANAGER
struct_ip_port_manager::struct_ip_port_manager(uint32_t _ip, uint16_t _port)
{
	ip = _ip;
	port = _port;
}

struct_ip_port_manager::struct_ip_port_manager(std::pair<uint32_t, uint16_t>& _in)
{
	ip = _in.first;
	port = _in.second;
}

struct_ip_port_manager::struct_ip_port_manager()
{
	ip = 0;
	port = 0;
}

bool struct_ip_port_manager::is_null()
{
	return ip == 0 && port == 0;
}

struct_route_header::struct_route_header()
{
	route_type = route_assigned;
}
