#include "intercall_methods.h"

#include <wpp/net/ip/socket/defines.h>
#include <wpp/hash/simple_encode.h>
#include <wpp/net/ip/convert.hpp>


uint32_t intercall_methods::default_generate_hash(void * data_ptr, uint32_t data_len)
{
	const int MOD_ADLER = 65521;
	uint32_t a = 1, b = 0;
	int32_t index;

	for (index = 0; index < (int)data_len; ++index) {

		a = (a + ((unsigned char*)data_ptr)[index]) % MOD_ADLER;
		b = (b + a) % MOD_ADLER;

	}
	return (b << 16) | a;
}

bool intercall_methods::default_validate_token(uint64_t token)
{
	return false;
}

bool intercall_methods::default_validate(void * data, uint32_t data_len, uint32_t sid)
{
	return false;
}

bool intercall_methods::get_data_from_encapsulated(
	struct_route_manager* encapsulated_packet, 
	uint32_t encapsulated_len, 
	char** out_raw_packet, 
	uint32_t & out_raw_len
){
	uint32_t route_size = encapsulated_packet->get_this_route_size();
	out_raw_len = encapsulated_len - route_size;
	*out_raw_packet = (char*)encapsulated_packet + route_size;

	return true;
}

// Build packet to alias, has one bridge the flow is [CLIENT|1xBRIDGE|ALIAS]
std::shared_ptr<wpp::net::ip::socket::buffers::buffer_data> intercall_methods::build_packet_to_alias(
	char* real_data, 
	uint32_t& real_len, 
	std::shared_ptr<boost::asio::ip::udp::endpoint> _rcv_endpoint,
	const struct_ip_port_manager& this_endpoint,
	struct_route_manager* packet, 
	const uint32_t count_route
){
	boost::scoped_array<struct_ip_port_manager> route_table{new struct_ip_port_manager[count_route]};
	// Riding route
	// First add client endpoint
	route_table.get()[0].ip		= _rcv_endpoint->address().to_v4().to_ulong();
	route_table.get()[0].port	= _rcv_endpoint->port();
	

	// After add this endpoint
	route_table.get()[1].ip		= this_endpoint.ip;
	route_table.get()[1].port	= this_endpoint.port;


	// And last, add current destination endpoint
	route_table.get()[2].ip		= packet->route_table[0].ip;
	route_table.get()[2].port	= packet->route_table[0].port;

	memcpy(&(route_table.get())[2], packet->route_table, sizeof(struct_ip_port_manager));
	// Make auth token default
	std::string empty_token;
	empty_token.resize(24);

	// Make a packet
	auto generated_packet = make_encapsulate_packet_raw_route(
		real_data,
		real_len,
		route_table.get(),
		count_route,
		2, /* skip client endpoint and this endpoint */
		empty_token, 
		packet->route_type
	);

	return generated_packet;
}

std::shared_ptr<wpp::net::ip::socket::buffers::buffer_data> 
intercall_methods::build_packet_to_bridge(
	char* real_data,
	uint32_t& real_len, 
	std::shared_ptr<boost::asio::ip::udp::endpoint> _rcv_endpoint, 
	struct_route_manager* packet, 
	const uint32_t count_route
){
	boost::scoped_array<struct_ip_port_manager> route_table{ new struct_ip_port_manager[count_route] };

	memcpy(&(route_table.get())[1], packet->route_table, sizeof(struct_ip_port_manager) * packet->total_route_index);

	route_table.get()->ip = _rcv_endpoint->address().to_v4().to_ulong();
	route_table.get()->port = _rcv_endpoint->port();

	// Make auth token default
	std::string empty_token;
	empty_token.resize(24);

	// Make a packet
	auto generated_packet = make_encapsulate_packet_raw_route(
		real_data,
		real_len,
		route_table.get(),
		count_route,
		2, /* skip client endpoint and this endpoint */
		empty_token,
		packet->route_type
	);

	return generated_packet;
}

std::shared_ptr<wpp::net::ip::socket::buffers::buffer_data> 
intercall_methods::make_encapsulate_packet_raw_route(
	void* real_data,
	const uint32_t& real_len, 
	struct_ip_port_manager* endpoints,
	const uint32_t route_count, 
	const uint32_t current_route_index, 
	const std::string& auth_token, 
	route_t route_type
){
	if (route_count <= 1) {
		LOGD("route count <= 1");
		return std::shared_ptr< wpp::net::ip::socket::buffers::buffer_data>();
	}

	// Packet default + sizeof endpoint * how many endpoint ( -1 because [0] )
	uint32_t route_packet_len = sizeof(struct_route_manager) + (sizeof(struct_ip_port_manager)* (route_count - 1));

	// Create buffer based on len packet + payload
	auto retval
		= std::make_shared<wpp::net::ip::socket::buffers::buffer_data>(route_packet_len + real_len);
	
	// Transform buffer data in struct_route_manager to set values on variables.
	// This could be done by using the pointer and adding the sizeof each variable.
	// Example retval.get()->get_data_ptr() + sizeof(uint32_t) for hash. But is unnecessary
	struct_route_manager* route_packet 
		= new (retval.get()->get_data_ptr())struct_route_manager(auth_token, current_route_index, route_count, route_type, endpoints);
	
	route_packet->update_route_hash();

	// Set payload
	memcpy(retval.get()->get_data_ptr() + route_packet_len, real_data, real_len);
	
	return retval;
}

wpp::serialization::json::json_value intercall_methods::encrypted_packet_to_json(std::string hct)
{
	hct = wpp::hash::simple_encode::decrypt_string(hct);
	auto _hct_vector = get_vector_from_packet(
		(uint8_t*)hct.data(),
		hct.size()
	);

	wpp::serialization::json::json_value _retval;
	for (auto& it : _hct_vector) {
		wpp::serialization::json::json_value _now;
		_now["address"] = wpp::net::ip::v4::convert::long_to_string(it.first);
		_now["tcp_port"] = it.second;
		_retval.append(_now);
	}

	return _retval;
}

std::vector<std::pair<uint32_t, uint16_t>> intercall_methods::get_vector_from_packet(uint8_t * _in, uint32_t len)
{
	std::vector< std::pair<uint32_t, uint16_t> >  retval;
	if (len % 6) {
		return retval;
	}

	uint32_t endpoints_count = len / 6/* 2 bytes port, 4 bytes ip*/;
	for (uint32_t i = 0; i < endpoints_count; i++) {
		uint32_t _ip_addr;
		uint16_t _port;

		_ip_addr = *(uint32_t*)(_in + (i * 6));
		_port = *(uint16_t*)(_in + (i * 6 + 4));

		retval.push_back({ _ip_addr, _port });
	}
	return retval;
}
