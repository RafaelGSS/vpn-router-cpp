#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <boost/thread/mutex.hpp>

#include <src/socket/packet_manipulation/memory_chunk.h>
#include <src/socket/tcp/connection/tcp_reproxy_basic.h>
#include <src/socket/interfaces/connection_settings.h>

#include <memory>

// Check if need timeout

class tcp_connection : public std::enable_shared_from_this<tcp_connection>
{
private:
	std::shared_ptr<boost::asio::ip::tcp::socket> connection;
	
	// I do not get it yet. But I think it's about when to close the main connection, upload that connection.
	std::shared_ptr<tcp_reproxy_basic> reproxy_server;

	// Connection information received after in handler first packet
	std::shared_ptr<connection_settings> config;

	boost::mutex _mtx;

	void process_to_alias();
	void process_to_bridge(
		const std::shared_ptr<boost::asio::ip::tcp::endpoint>& next_endpoint
	);

	// Check if next hop is other bridge or final server.
	void process_hops_connection();

protected:
	// Check if is reproxy received from payload or hops
	void process_connection();

	// Send a status to connection for possible close connection
	void send_status(
		const std::shared_ptr<boost::asio::ip::tcp::socket>& socket,
		const uint32_t status,
		bool close_con
	);

	void connect_to_destination(
		const std::string& address,
		const uint16_t& port
	);
public:
	bool is_closed();

	void close_connection();

	void initialize_first_receive();

	tcp_connection(
		const std::shared_ptr<boost::asio::ip::tcp::socket>& conn
	);

	/*
		Handlers
	*/

	void handler_first_packet_size(
		const std::shared_ptr<tcp_connection>& holder, 
		const boost::system::error_code& ec,
		size_t bytes_transferred, 
		const std::shared_ptr<memory_chunk>& buff
	);

	void handler_first_packet(
		const std::shared_ptr<tcp_connection>& holder,
		const boost::system::error_code& ec,
		const size_t& bytes_transferred,
		const std::shared_ptr<memory_chunk>& buff
	);

	void handler_send_status(
		const std::shared_ptr<tcp_connection>& holder,
		const size_t& bytes_transferred,
		bool close_con
	);

	/* Handler to destination */

	void handler_connected_on_destination(
		const std::shared_ptr<tcp_connection>& holder,
		const boost::system::error_code& error
	);

	void handler_send_header_on_destination(
		const std::shared_ptr<tcp_connection>& holder,
		size_t bytes_transferred,
		const boost::system::error_code& error,
		const std::shared_ptr<wpp::net::ip::socket::buffers::buffer_data>& buffer
	);

	void handler_recv_status_on_destination(
		const std::shared_ptr<tcp_connection>& holder,
		size_t bytes_transferred,
		const boost::system::error_code& error,
		const std::shared_ptr<wpp::net::ip::socket::buffers::buffer_data>& buffer
	);

	void handler_send_ready_on_destination(
		const std::shared_ptr<tcp_connection>& holder,
		size_t bytes_transferred,
		const boost::system::error_code& error,
		const std::shared_ptr<wpp::net::ip::socket::buffers::buffer_data>& buffer
	);

	/* After payload security */

	void set_listen_client();
	void set_listen_server();

	/* Handler connection after security */
	void handler_receive_client(
		const std::shared_ptr<tcp_connection>& holder,
		size_t bytes_transferred,
		const boost::system::error_code& error,
		const std::shared_ptr<memory_chunk>& buffer
	);

	void handler_receive_server(
		const std::shared_ptr<tcp_connection>& holder,
		size_t bytes_transferred,
		const boost::system::error_code& error,
		const std::shared_ptr<memory_chunk>& buffer
	);

	void handler_write_server(
		const std::shared_ptr<tcp_connection>& holder,
		size_t bytes_transferred,
		const boost::system::error_code& error,
		const std::shared_ptr<memory_chunk>& buffer
	);

	void handler_write_client(
		const std::shared_ptr<tcp_connection>& holder,
		size_t bytes_transferred,
		const boost::system::error_code& error,
		const std::shared_ptr<memory_chunk>& buffer
	);


	~tcp_connection();
};

