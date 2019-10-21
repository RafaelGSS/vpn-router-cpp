#pragma once
#ifndef __linux__
	#include <wpp/windows/include_fix/winsock.hpp>
	#define _WIN32_WINNT_WIN10                  0x0A00 // Windows 10 
#endif

// WPP INCLUDES
#include <wpp/settings/conf.hpp>
#include <wpp/console/console.hpp>

// CURRENT PROJECT INCLUDES
#include <src/controller/bridge_manager.h>
#include <src/http/remote/base/remote_api_base.h>
#include <src/database/base/data_access.h>
#include <config.h>

// STANDARD INCLUDES
#include <exception>
#include <iostream>
#include <cstdio>
#include <memory>

void init_others() {
	// Initialize configurations on singleton file_bridge_manager
	remote_api_base::get_default()->init();

	// Use the informations on singleton file_bridge_manager
	data_access::get_default()->run();
}

int main()
{
	std::shared_ptr<bridge_manager> bridge = std::make_shared<bridge_manager>();
#ifdef __linux__
	//wpp::console::parse_args_execute();
#endif
	init_others();
	//wpp::settings::priority::elevate_priority(2);

	bridge->run();

	while (true)
		sleep_mili((uint32_t)-1);
	
	LOGE("Something has ocurred");
	return 0;
}