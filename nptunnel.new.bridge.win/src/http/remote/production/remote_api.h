#pragma once
#include <src/settings/file_managment/file_manager.h>
#include <src/http/remote/base/remote_api_base.h>

#include <condition_variable>
#include <mutex>
#include <string>

#include <wpp/serialization/json.hpp>

class remote_api : 
	public remote_api_base
{
protected:
	bool remote_config_loaded;
	bool token_accepted;

	std::string config_url;

	std::condition_variable cv;
	std::mutex mtx;

	virtual void get_remote_config() override;
	virtual void start_update_thread() override;
	virtual void start_check_update_alias_thread();

	virtual void wait_first_config() override;
	virtual void wait_token_config();

	virtual void check_update();

	void on_request_remote_config(uint32_t error_code, std::string& response);
	void on_request_check_update(uint32_t error_code, std::string& response);
	
	void set_remote_accepted();
private:
	void get_token_to_file();

public:
	virtual void init() override;
	remote_api();
};

