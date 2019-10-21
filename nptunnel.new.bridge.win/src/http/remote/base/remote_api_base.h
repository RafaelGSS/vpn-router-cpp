#pragma once
class remote_api_base
{
public:
	virtual void get_remote_config() = 0;
	virtual void start_update_thread() = 0;
	virtual void wait_first_config() = 0;
	virtual void init() = 0;

	remote_api_base();
	static remote_api_base* get_default();
};

