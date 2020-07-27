#pragma once
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

class tcp_reproxy_basic
{
	boost::asio::io_service& io_service;

public:
	std::shared_ptr<boost::asio::ip::tcp::socket> _sock;

	void close();
	void bind(
		boost::asio::ip::tcp::endpoint _ep
	);
	void set_no_delay();

	tcp_reproxy_basic(boost::asio::io_service& _io_service);
	~tcp_reproxy_basic();
};

