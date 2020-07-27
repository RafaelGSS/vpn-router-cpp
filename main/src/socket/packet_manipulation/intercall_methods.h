#pragma once
#include <stdint.h>
#include <vector>
#include <map>

#include <src/socket/interfaces/struct_route.h>

#include <boost/scoped_array.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <wpp/net/ip/socket/buffers.hpp>
#include <wpp/serialization/json.hpp>

class intercall_methods
{
public:
	/**
	*	UDP Methods
	**/

	static uint32_t default_generate_hash(void* data_ptr, uint32_t data_len);
	static bool default_validate_token(uint64_t token);

	static bool default_validate(void* data, uint32_t data_len, uint32_t sid);

	static bool get_data_from_encapsulated(
		struct_route_manager* encapsulated_packet,
		uint32_t encapsulated_len,
		char** out_raw_packet,
		uint32_t& out_raw_len
	);

	static std::shared_ptr<wpp::net::ip::socket::buffers::buffer_data> build_packet_to_alias(
		char* real_data,
		uint32_t& real_len,
		std::shared_ptr<boost::asio::ip::udp::endpoint> _rcv_endpoint,
		const struct_ip_port_manager& this_endpoint,
		struct_route_manager* packet,
		const uint32_t count_route
	);

	static std::shared_ptr<wpp::net::ip::socket::buffers::buffer_data> build_packet_to_bridge(
		char* real_data,
		uint32_t& real_len,
		std::shared_ptr<boost::asio::ip::udp::endpoint> _rcv_endpoint,
		struct_route_manager* packet,
		const uint32_t count_route
	);

	static std::shared_ptr<wpp::net::ip::socket::buffers::buffer_data> make_encapsulate_packet_raw_route(
		void* real_data,
		const uint32_t& real_len,
		struct_ip_port_manager* endpoints,
		const uint32_t route_count,
		const uint32_t current_route_index,
		const std::string& auth_token,
		route_t route_type
	);

	/**
	*	TCP Methods
	**/

	static wpp::serialization::json::json_value encrypted_packet_to_json(
		std::string hct
	);

	static std::vector<std::pair<uint32_t, uint16_t>> get_vector_from_packet(
		uint8_t* _in,
		uint32_t len
	);
};
