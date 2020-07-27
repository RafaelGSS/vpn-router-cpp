#include "tcp_socket.h"

#include <src/socket/tcp/connection/tcp_connection.h>

#include <memory>

void tcp_socket::handler_accept(
	const std::shared_ptr<boost::asio::ip::tcp::socket>& client, 
	const boost::system::error_code & error
){
	start_accept();
	if (error) {
		LOGD(error.message());
		close_socket(client);
		return;
	}
	client->set_option(boost::asio::ip::tcp::no_delay(true));
	auto new_connection = std::make_shared<tcp_connection>(client);
	new_connection->initialize_first_receive();
}

void tcp_socket::handler_recv(
	const std::shared_ptr<boost::asio::ip::tcp::socket>& client,
	const boost::system::error_code & error, 
	size_t bytes_transferred, 
	const std::string & buffer
){
}

void tcp_socket::handler_send(
	const std::shared_ptr<boost::asio::ip::tcp::socket>& server, 
	const boost::system::error_code & error,
	size_t bytes_transferred
){
}

void tcp_socket::handler_connect(
	const std::shared_ptr<boost::asio::ip::tcp::socket>& server,
	const std::shared_ptr<boost::asio::ip::tcp::endpoint>& endpoint, 
	const boost::system::error_code & error
){
}

tcp_socket::tcp_socket(
	uint16_t port, 
	std::string this_server_bind
) :
	socket_tcp(std::move(this_server_bind), port)
{
}

tcp_socket::~tcp_socket()
{
}
