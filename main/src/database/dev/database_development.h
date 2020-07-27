#pragma once
#include <src/database/base/data_access.h>

class database_development :
	public data_access
{
public:
	virtual void run() override;

	virtual alias_info get_alias_info(uint32_t alias_id) override;
	virtual bool validate_user(char* token, char* machine_uid, uint32_t sid) override;

	virtual void update_users_cache();
	virtual void update_alias_cache();

	database_development();
	~database_development();
};

