#include "alias_info.h"
#include <boost/asio/ip/address_v4.hpp>

#include <wpp/net/ip/socket/defines.h>
#include <wpp/net/ip/convert.hpp>

std::string alias_info::ipv4_address_string()
{
	return boost::asio::ip::address_v4(ipv4_address).to_string();
}

void alias_info::set_ipv4_address(std::string str_ip)
{
	try
	{
		ipv4_address = boost::asio::ip::address_v4::from_string(str_ip).to_ulong();
	}
	catch (std::exception&) {
		LOGE("On set ipv4 from string");
		ipv4_address = 0;
	}
}

bool alias_info::is_null_tcp()
{
	return tcp_port == 0;
}

bool alias_info::is_null_udp()
{
	return udp_port == 0;
}

bool alias_info::is_null_address()
{
	return ipv4_address == 0;
}

alias_info::alias_info() :
	ipv4_address(0), udp_port(0), tcp_port(0)
{}

alias_info::alias_info(uint32_t _ipv4_address, uint16_t _tcp_port, uint16_t _udp_port) :
	ipv4_address(_ipv4_address), tcp_port(_tcp_port), udp_port(_udp_port)
{}

alias_info::alias_info(
	std::string ip,
	std::string _tcp_port, 
	std::string _udp_port
) : ipv4_address(wpp::net::ip::v4::convert::string_to_long(ip)),
	tcp_port(std::atoi(_tcp_port.data())), udp_port(std::atoi(_udp_port.data()))
{}
