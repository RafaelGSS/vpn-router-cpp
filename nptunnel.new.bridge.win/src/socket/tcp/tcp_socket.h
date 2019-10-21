#pragma once
#include <stdint.h>

#include <wpp/net/ip/socket/socket_tcp.hpp>

class tcp_socket
	: public wpp::net::ip::socket::tcp::socket_tcp
{
public:
	virtual void handler_accept(
		const std::shared_ptr<boost::asio::ip::tcp::socket>& client,
		const boost::system::error_code& error
	) override;

	virtual void handler_recv(
		const std::shared_ptr<boost::asio::ip::tcp::socket>& client,
		const boost::system::error_code& error,
		size_t bytes_transferred,
		const std::string& buffer
	) override;

	virtual void handler_send(
		const std::shared_ptr<boost::asio::ip::tcp::socket>& server,
		const boost::system::error_code& error,
		size_t bytes_transferred
	) override;

	virtual void handler_connect(
		const std::shared_ptr<boost::asio::ip::tcp::socket>& server,
		const std::shared_ptr<boost::asio::ip::tcp::endpoint>& endpoint,
		const boost::system::error_code& error
	) override;

	// If passed this_server_bind empty, will pick up local network = 0.0.0.0
	tcp_socket(
		uint16_t port,
		std::string this_server_bind
	);

	~tcp_socket();
};

