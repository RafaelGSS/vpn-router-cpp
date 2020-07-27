#pragma once
#include <chrono>
#include <thread>

#define SERVER_CONF_FILE "bridge.conf"
#ifndef sleep_mili
#define sleep_mili(t) std::this_thread::sleep_for(std::chrono::milliseconds(t))
#endif

#define DEFAULT_MEMORY_CHUNCK 15000

#define STUFF_VERSION "RICHS_1"


#define LOCAL_TCP_PORT 1000
#define LOCAL_UDP_PORT 1001



/*
	REMOTE URI CONFIGURATIONS
*/

#define PREFIX_API "/api/bridge"

#define URL_CHECK_UPDATE  PREFIX_API "/tasks"
#define URL_UPDATE_ID  PREFIX_API "/task/completed"
#define URL_GET_TOKEN PREFIX_API "/token"
#define URL_GET_ALIAS PREFIX_API "/alias/all"
#define URL_GET_INFO PREFIX_API "/info"


#define VERSION "2.1.3"