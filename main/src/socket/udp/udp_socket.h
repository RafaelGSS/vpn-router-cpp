#include <src/socket/interfaces/struct_route.h>


class udp_socket :
	public wpp::net::ip::socket::udp::socket_udp
{
private:
	boost::mutex mtx;

	void from_client_process_first_packet_to_bridge(
		char* real_data,
		uint32_t& real_len,
		struct_route_manager* packet,
		uint32_t packet_len,
		const std::shared_ptr<boost::asio::ip::udp::endpoint>& _rcv_endpoint
	);

	void from_client_process_first_packet_to_alias(
		char* real_data,
		uint32_t& real_len,
		struct_route_manager* packet,
		uint32_t& packet_len,
		const std::shared_ptr<boost::asio::ip::udp::endpoint>& _rcv_endpoint
	);

protected:

	void process_packet(
		uint8_t* data,
		uint32_t data_len, 
		const std::shared_ptr<boost::asio::ip::udp::endpoint>& _rcv_endpoint
	);

	void process_hop_from_client(
		struct_route_manager* packet,
		uint32_t packet_len,
		const std::shared_ptr<boost::asio::ip::udp::endpoint>& _rcv_endpoint
	);

	void process_hop_from_server(
		struct_route_manager* packet,
		uint32_t packet_len,
		const std::shared_ptr<boost::asio::ip::udp::endpoint>& _rcv_endpoint
	);

	void from_client_process_first_route(
		struct_route_manager* packet,
		uint32_t& packet_len,
		const std::shared_ptr<boost::asio::ip::udp::endpoint>& _rcv_endpoint
	);

	void from_client_process_next_route(
		struct_route_manager* packet,
		uint32_t packet_len
	);

public:
	udp_socket(
		uint16_t port,
		std::string server,
		std::string address
	);

	virtual void handle_recv(
		const boost::system::error_code& ec,
		const size_t& bytes_transferred,
		const std::shared_ptr<boost::array<uint8_t, BRIDGE_BUFFER_SIZE>>& buffer,
		const std::shared_ptr<boost::asio::ip::udp::endpoint>& _rcv_endpoint
	);

	virtual void handle_send(
		const boost::system::error_code& ec,
		size_t bytes_transferred,
		const std::string& data
	);
};

