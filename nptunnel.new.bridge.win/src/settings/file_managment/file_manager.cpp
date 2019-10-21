#include "file_manager.h"
#include <config.h>
#include <src/settings/file_managment/settings.h>

#include <boost/filesystem.hpp>

#include <wpp/serialization/json.hpp>

file_manager::file_manager(std::string fileName)
{
	fileParsed = settings::load_file_config_parsed(fileName);
}

file_manager_bridge_base * file_manager_bridge_base::singleton(std::string fileName)
{
	static file_manager_bridge_base* singleton = nullptr;
	if (!singleton) {
#ifdef TEST_API
		singleton = new file_bridge_manager_offline(fileName);
#else
		singleton = new file_bridge_manager(fileName);
#endif
	}
	return singleton;
}
file_manager_bridge_base::file_manager_bridge_base(std::string fileName) :
	file_manager(std::move(fileName))
{

}

/**
		ONLINE MODE ( PRODUCTION MODE )
**/

file_bridge_manager::file_bridge_manager(std::string fileName) : 
	file_manager_bridge_base(fileName)
{
	
}

std::string file_bridge_manager::get_config_url()
{
	return this->get("config_url");
}

std::string file_bridge_manager::get_bind_address()
{
	return this->get("bind_ip");
}

std::string file_bridge_manager::get_address()
{
	return this->get("ip");
}

std::string file_bridge_manager::get_name()
{
	return this->get("name");
}

int32_t file_bridge_manager::get_core_afinnity_num()
{
	return this->get_value<uint32_t>("core", 1);
}

uint16_t file_bridge_manager::get_udp_port()
{
	return this->get_value<uint16_t>("port_udp");
}

uint16_t file_bridge_manager::get_tcp_port()
{
	return this->get_value<uint16_t>("port_tcp");
}

void file_bridge_manager::set_token(std::string value)
{
	set("token", value);
}

std::string file_bridge_manager::get_token()
{
	return get("token");
}


/**

		OFFLINE MODE! ( WHEN SET TEST_API=1 ON PRE-PROCESSOR )

**/

file_bridge_manager_offline::file_bridge_manager_offline(std::string fileName) :
	file_manager_bridge_base(fileName)
{

}

std::string file_bridge_manager_offline::get_config_url()
{
	return std::string();
}

std::string file_bridge_manager_offline::get_bind_address()
{
	return "0.0.0.0";
}

std::string file_bridge_manager_offline::get_address()
{
	return "0.0.0.0";
}

std::string file_bridge_manager_offline::get_name()
{
	return std::string();
}

int32_t file_bridge_manager_offline::get_core_afinnity_num()
{
	return 1;
}

uint16_t file_bridge_manager_offline::get_udp_port()
{
	return get_value<uint16_t>("udp_port", LOCAL_UDP_PORT);
}

uint16_t file_bridge_manager_offline::get_tcp_port()
{
	return get_value<uint16_t>("tcp_port", LOCAL_TCP_PORT);
}

void file_bridge_manager_offline::set_token(std::string value)
{
}

std::string file_bridge_manager_offline::get_token()
{
	return "AAAAAAAAAAAAA";
}
