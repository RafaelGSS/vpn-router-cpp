#include "udp_socket.h"

#include <src/socket/packet_manipulation/intercall_methods.h>
#include <src/database/base/data_access.h>

#include <wpp/net/ip/socket/buffers.hpp>

#include <boost/asio/placeholders.hpp>

void udp_socket::from_client_process_first_packet_to_bridge(
	char* real_data,
	uint32_t& real_len,
	struct_route_manager* packet,
	uint32_t packet_len,
	const std::shared_ptr<boost::asio::ip::udp::endpoint>& _rcv_endpoint
){
	uint32_t new_route_count = packet->total_route_index + 1; /* client endpoint */

	auto packet_to_bridge = intercall_methods::build_packet_to_bridge(
		real_data, real_len,
		_rcv_endpoint,	packet, 
		new_route_count
	);

	auto packet_struct_ptr = ((struct_route_manager*)packet_to_bridge.get()->get_data_ptr());
	auto next_endpoint = packet_struct_ptr->get_current_route();

	// if next is alias
	if (!packet_struct_ptr->has_next_route() || next_endpoint.port == 0) {
		auto info = data_access::get_default()->get_alias_info(next_endpoint.ip);
		if (info.is_null_udp()) {
			std::string generic_message = "Udp null from alias : " + next_endpoint.ip;
			LOGE(generic_message);
			return;
		}

		if (next_endpoint.port == 0) {
			next_endpoint.ip = info.ipv4_address;
			next_endpoint.port = info.udp_port;
		}
		else {
			next_endpoint.ip = info.ipv4_address;
		}
	}

	send_packet_to(packet_to_bridge, next_endpoint.ip, next_endpoint.port);
}

void udp_socket::from_client_process_first_packet_to_alias(
	char* real_data,
	uint32_t& real_len,
	struct_route_manager* packet,
	uint32_t& packet_len,
	const std::shared_ptr<boost::asio::ip::udp::endpoint>& _rcv_endpoint
){
	uint32_t new_route_count = packet->total_route_index + 2; /* client endpoint + this endpoint */

	auto packet_to_alias = intercall_methods::build_packet_to_alias(
		real_data, real_len,
		_rcv_endpoint, { server_ip_ulong , udp_port },
		packet, new_route_count
	);

	auto packet_struct_ptr = ((struct_route_manager*)packet_to_alias.get()->get_data_ptr());
	auto next_endpoint = packet_struct_ptr->get_current_route();
	// All alias has port and ip undefinned for security. 
	// So, if next route is alias neeed resolve alias destination
	if (!packet->has_next_route() || next_endpoint.port == 0) {

		auto info = data_access::get_default()->get_alias_info(next_endpoint.ip);
		if (info.is_null_udp()) {
			std::string generic_message = "Udp null from alias: " + std::to_string(next_endpoint.ip);
			LOGE(generic_message);
			return;
		}

		if (next_endpoint.port == 0) {
			next_endpoint.ip = info.ipv4_address;
			next_endpoint.port = info.udp_port;
		}
		else {
			next_endpoint.ip = info.ipv4_address;
		}
	}

	send_packet_to(packet_to_alias, next_endpoint.ip, next_endpoint.port);
}

void udp_socket::process_packet(
	uint8_t * data, 
	uint32_t data_len, 
	const std::shared_ptr<boost::asio::ip::udp::endpoint>& _rcv_endpoint
){
	struct_route_manager* route_packet = (struct_route_manager*)data;
	if (!route_packet->is_route_valid(data_len)) {
		LOGE("error on validation");
		return;
	}

	switch (route_packet->route_type)
	{
	case route_hop_from_client:
		process_hop_from_client(route_packet, data_len, _rcv_endpoint);
		break;
	case route_hop_from_server:
		process_hop_from_server(route_packet, data_len, _rcv_endpoint);
		break;
	default:
		LOGE("route packet type invalid");
		break;
	}
}

void udp_socket::process_hop_from_client(
	struct_route_manager * packet,
	uint32_t packet_len, 
	const std::shared_ptr<boost::asio::ip::udp::endpoint>& _rcv_endpoint
){
	boost::lock_guard<boost::mutex> lock(mtx);


	// Check if is first bridge received packet
	if (packet->current_route_index == 0) {
		from_client_process_first_route(packet, packet_len, _rcv_endpoint);
	}
	else {
		from_client_process_next_route(packet, packet_len);
	}
}

void udp_socket::process_hop_from_server(
	struct_route_manager * packet,
	uint32_t packet_len, 
	const std::shared_ptr<boost::asio::ip::udp::endpoint>& _rcv_endpoint
){
	boost::lock_guard<boost::mutex> lock(mtx);

	auto data = (uint8_t*)packet;

	// Advance and update hops index
	packet->update_route_index();
	packet->update_route_hash();
	
	// Get next destination of packet and build a payload
	auto destination = packet->get_current_route();
	auto _payload = wpp::net::ip::socket::buffers::create_buffer_shared(data, packet_len);

	send_packet_to(_payload, destination.ip, destination.port);
}

void udp_socket::from_client_process_first_route(
	struct_route_manager* packet,
	uint32_t& packet_len,
	const std::shared_ptr<boost::asio::ip::udp::endpoint>& _rcv_endpoint
) {
	// Retrieve data whithout routing headers
	char* real_data;
	uint32_t real_len;

	if (!intercall_methods::get_data_from_encapsulated(packet, packet_len, &real_data, real_len)) {
		LOGD("data from encapsulated");
		return;
	}
	// If next is alias, we need pass the route for him
	// Client endpoint --> This bridge endpoint --> Alias endpoint
	if (packet->total_route_index == 1) {
		from_client_process_first_packet_to_alias(real_data, real_len, packet, packet_len, _rcv_endpoint);
	}
	else {
		from_client_process_first_packet_to_bridge(real_data, real_len, packet, packet_len, _rcv_endpoint);
	}
}

void udp_socket::from_client_process_next_route(
	struct_route_manager* packet,
	uint32_t packet_len
){
	auto destination = packet->get_next_route();

	//advance route index and update hash
	packet->update_route_index();
	packet->update_route_hash();

	if (!packet->has_next_route() || destination.port == 0) {
		auto info = data_access::get_default()->get_alias_info(destination.ip);
		if (info.is_null_udp()) {
			LOGE("udp null");
			return;
		}

		if (destination.port == 0) {
			destination.ip = info.ipv4_address;
			destination.port = info.udp_port;
		}
		else {
			destination.ip = info.ipv4_address;
		}
	}

	auto data = (uint8_t*)packet;
	auto packet_to_next = wpp::net::ip::socket::buffers::create_buffer_shared(data, packet_len);

	send_packet_to(packet_to_next, destination.ip, destination.port);
}

udp_socket::udp_socket(uint16_t port, std::string server_ip, std::string address_bind) :
	socket_udp(port, server_ip, address_bind)
{
}
void udp_socket::handle_recv(
	const boost::system::error_code & ec,
	const size_t & bytes_transferred, 
	const std::shared_ptr<boost::array<uint8_t, BRIDGE_BUFFER_SIZE>>& buffer,
	const std::shared_ptr<boost::asio::ip::udp::endpoint>& _rcv_endpoint
){
	if (ec) {
		LOGD(ec.message());
		set_recv_from();
		return;
	}

	process_packet(buffer->data(), bytes_transferred, _rcv_endpoint);
	set_recv_from();
}

void udp_socket::handle_send(
	const boost::system::error_code & ec,
	size_t bytes_transferred,
	const std::string & data
){
	if (ec) {
		LOGD(ec.message());
	}
}
