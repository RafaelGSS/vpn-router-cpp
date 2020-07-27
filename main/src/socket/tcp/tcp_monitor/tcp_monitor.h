#pragma once
#include <stdint.h>
#include <mutex>

class tcp_monitor
{
private:
	tcp_monitor();
	uint32_t users_tcp;
	std::mutex _mtx;

public:
	uint32_t get_users_tcp();

	void new_user();
	void close_user();

	static tcp_monitor* get();
};

