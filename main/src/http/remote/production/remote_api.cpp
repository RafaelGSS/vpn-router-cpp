#include "remote_api.h"

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

#include <config.h>
#include <src/settings/validator/validator_http.h>
#include <src/settings/file_managment/settings.h>
#include <src/http/requester/http_request.h>
#include <src/database/base/data_access.h>
#include <src/socket/tcp/tcp_monitor/tcp_monitor.h>

#include <wpp/net/ip/socket/defines.h>



void remote_api::get_remote_config()
{
	
	WPP_JVALUE args;
	args["token"] = file_bridge_manager::singleton()->get_token();

	auto _url = config_url + URL_GET_INFO;

	http_request::get()->do_request(_url,
		std::bind(
			&remote_api::on_request_remote_config,
			this,
			std::placeholders::_1,
			std::placeholders::_2
		), args, true);
}

void remote_api::on_request_remote_config(uint32_t error_code, std::string& response)
{
	if (!validator_http::is_valid_response(response, error_code)) {
		LOGD(error_code);
		return;
	}

	auto _settings_remote = wpp::serialization::json::parse(response).to_map();
	if (!_settings_remote.size()) {
		LOGD("parsing response to map error");
		return;
	}

	for (auto& _kv : _settings_remote)
		file_manager_bridge_base::singleton()->set(_kv.first, _kv.second);
	set_remote_accepted();
}

void remote_api::on_request_check_update(uint32_t error_code, std::string & response)
{
	// Check need update now alias
	if (!validator_http::is_valid_response(response, error_code)) {
		LOGD(error_code);
		return;
	}

	auto _response = wpp::serialization::json::parse(response);
	bool update = false;
	bool desativate = false;

	for (auto& _kv : _response)
	{
		uint32_t id = std::atoi(_kv["id"].asString().data());
		
		auto _url = config_url + URL_UPDATE_ID;

		WPP_JVALUE args;
		args["token"] = file_bridge_manager::singleton()->get_token();
		args["id_task"] = id;

		http_request::get()->do_request(_url,
			[](uint32_t ec, std::string& res) {},
			args, true);

		if (_kv["task"] == "update")
			update = true;
		if (_kv["task"] == "close")
			desativate = true;
	}
	if (update) 
		data_access::get_default()->update_alias_cache();
	
	if (desativate)
		exit(0);
}

void remote_api::start_update_thread()
{
	auto _update_thread = [&] {
		// Get configurations from panel
		while (true)
		{
			get_remote_config();
			sleep_mili(std::chrono::minutes(5));
		}
	};

	boost::thread(_update_thread).detach();
}

void remote_api::start_check_update_alias_thread()
{
	auto _update_thread = [&] {
		// Send request each 5 min for check new updates
		while (true)
		{
			check_update();
			sleep_mili(std::chrono::minutes(5));
		}
	};

	boost::thread(_update_thread).detach();
}

void remote_api::wait_first_config()
{
	std::unique_lock<std::mutex> lock(mtx);
	cv.wait(lock, [&] {
		return remote_config_loaded;
	});
}

void remote_api::wait_token_config()
{
	std::unique_lock<std::mutex> lock(mtx);
	cv.wait(lock, [&] {
		return token_accepted;
	});
}

void remote_api::check_update()
{
	WPP_JVALUE args;
	args["token"] = file_manager_bridge_base::singleton()->get_token();
	args["version"] = VERSION;
	args["users_tcp"] = tcp_monitor::get()->get_users_tcp();

	auto _url = config_url + URL_CHECK_UPDATE;

	http_request::get()->do_request(_url,
		std::bind(
			&remote_api::on_request_check_update,
			this,
			std::placeholders::_1,
			std::placeholders::_2
		), args, true);
}

void remote_api::set_remote_accepted()
{
	remote_config_loaded = true;
	cv.notify_all();
}

void remote_api::get_token_to_file()
{
	auto _url = config_url + URL_GET_TOKEN;
	auto lambda = [&](uint32_t error_code, std::string& response) {
		if (!validator_http::is_valid_response(response, error_code)) {
			LOGE("error on recept token");
			return;
		}

		auto jres = wpp::serialization::json::parse(response).to_map();
		if (!jres.size()) {
			LOGE("error parsing token");
			return;
		}

		file_manager_bridge_base::singleton()->set_token(jres["token"]);
		token_accepted = true;
		LOGI(file_manager_bridge_base::singleton()->get_token());

		cv.notify_all();
	};

	http_request::get()->do_request(_url, lambda, {}, true);
}


void remote_api::init()
{
	// Initialize singleton http_request 
	http_request::get();

	// Set token on singleton file manager.
	get_token_to_file();
	wait_token_config();
	// Initialize thread to check updates from panel
	//start_update_thread(); Not need thread yet. Test
	get_remote_config();
	wait_first_config();

	// Start thread to check if need update alias.
	start_check_update_alias_thread();
}

remote_api::remote_api() : remote_config_loaded(false)
{
	// Comunication with BD alias and validate user
	config_url = settings::decrypt_nphash(
		file_manager_bridge_base::singleton()->get_config_url(),
		std::atoi(file_manager_bridge_base::singleton()->get("key_token").data())
	);
}



