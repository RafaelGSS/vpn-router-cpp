#include "tcp_reproxy_basic.h"
#include <wpp/net/ip/socket/defines.h>

void tcp_reproxy_basic::close()
{
	//if (_sock->is_open()) {
		boost::system::error_code ignore_ec;
		_sock->cancel(ignore_ec);

		ignore_ec.clear();
		_sock->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignore_ec);
	
		ignore_ec.clear();
		_sock->close(ignore_ec);
	//}
}

void tcp_reproxy_basic::bind(boost::asio::ip::tcp::endpoint _ep)
{
	boost::system::error_code ec;
	if (!_sock->is_open()) {
		_sock->open(boost::asio::ip::tcp::v4());
	}
	_sock->bind(_ep, ec);

	if (ec)
		LOGD(ec.message());

	set_no_delay();
}

void tcp_reproxy_basic::set_no_delay()
{
	boost::system::error_code ignored_error;
	
	_sock->set_option(boost::asio::ip::tcp::no_delay(true),ignored_error);
	
	if (ignored_error)
		LOGD("error on set delay");
}

tcp_reproxy_basic::tcp_reproxy_basic(boost::asio::io_service& _io_service) : io_service(_io_service)
{
	_sock = std::make_shared<boost::asio::ip::tcp::socket>(_io_service);
}

tcp_reproxy_basic::~tcp_reproxy_basic()
{
}
