#pragma once
#include <src/http/remote/base/remote_api_base.h>

class local_api :
	public remote_api_base
{
protected:
	virtual void get_remote_config() override;
	virtual void start_update_thread() override;

	virtual void wait_first_config() override;
public:
	virtual void init() override;
	local_api();
};

