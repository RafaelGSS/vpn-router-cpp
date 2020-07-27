#include "database_development.h"
#include <wpp/net/ip/convert.hpp>


void database_development::run()
{
	
}


alias_info database_development::get_alias_info(uint32_t alias_id)
{
	if (alias_id < 100) {
		/*
		alias 0 = {127.0.0.1, tcpport : 2000, udpport : 2001}
		alias 1 = {127.0.0.1, tcpport : 2002, udpport : 2003}
		alias 2 = {127.0.0.1, tcpport : 2004, udpport : 2005}
		alias 3 = {127.0.0.1, tcpport : 2006, udpport : 2007}
		*/

		return alias_info(
			WPP_IP_ATOI("127.0.0.1"),
			(uint16_t)(2000 + (alias_id * 2)),
			(uint16_t)(2000 + (alias_id * 2) + 1)
		);
	}
	return alias_info();
}

bool database_development::validate_user(char * token, char * machine_uid, uint32_t sid)
{
	return true;
}

void database_development::update_users_cache()
{
}

void database_development::update_alias_cache()
{
}

database_development::database_development()
{
}


database_development::~database_development()
{
}
