#pragma once
#include <src/socket/interfaces/alias_info.h>

class data_access
{
public:
	virtual void run() = 0;

	virtual alias_info get_alias_info(
		uint32_t alias_id
	) = 0;

	virtual bool validate_user(
		char* token,
		char* machine_uid,
		uint32_t sid
	) = 0;

	virtual void update_users_cache() = 0;
	virtual void update_alias_cache() = 0;

	data_access();
	static data_access* get_default();
};

