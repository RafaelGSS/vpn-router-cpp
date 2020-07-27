#pragma once
#include <wpp/windows/include_fix/winsock.hpp>


#include <src/database/base/data_access.h>
#include <src/settings/file_managment/file_manager.h>
#include <src/socket/interfaces/user_info.h>

#include <condition_variable>
#include <memory>

class database_production : 
	public data_access
{
private:
	std::mutex mtx_alias;
	std::mutex mtx_users;
	std::condition_variable cv;

	std::string decrypted_token;
	std::string decrypted_url;
	int key_token;

	std::shared_ptr<std::map<uint32_t, alias_info>> alias_cache;
	std::shared_ptr<std::map<std::string, user_info>> users_cache;

	void thread_update_alias_cache();
	void thread_update_users_cache();

	void initialize();
	void wait_alias_cache_updated();
protected:
	virtual bool has_user_in_cache(const std::string& token);
	virtual bool is_valid_user(const std::string& token);

	virtual void request_validate_user(user_info& _new_user, bool now = true);

	virtual void handler_get_alias(uint32_t error, std::string& response);
public:
	virtual void run() override;
	
	virtual alias_info get_alias_info(uint32_t alias_id) override;
	
	virtual bool validate_user(
		char* token,
		char* machine_uid,
		uint32_t sid
	) override;
	
	virtual void update_users_cache() override;
	virtual void update_alias_cache() override;

	database_production();
};

/*
	This class has 2 threads running:
		- The first update alias_cache for each hour
		- The second update users_cache for each twelve hours
*/