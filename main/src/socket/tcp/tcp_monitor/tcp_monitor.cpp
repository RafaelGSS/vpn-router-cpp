#include "tcp_monitor.h"


uint32_t tcp_monitor::get_users_tcp()
{
	std::lock_guard<std::mutex> lock(_mtx);
	return users_tcp;
}

void tcp_monitor::new_user()
{
	std::lock_guard<std::mutex> lock(_mtx);
	users_tcp++;
}

void tcp_monitor::close_user()
{
	std::lock_guard<std::mutex> lock(_mtx);
	users_tcp--;
}

tcp_monitor::tcp_monitor()
{
	users_tcp = 0;
}

tcp_monitor * tcp_monitor::get()
{
	static tcp_monitor* singleton = nullptr;
	if (!singleton) {
		singleton = new tcp_monitor();
	}
	return singleton;
}

