#include "database_production.h"

#include <config.h>
#include <src/http/requester/http_request.h>
#include <src/settings/file_managment/settings.h>

#include <wpp/net/ip/socket/defines.h>
#include <wpp/thread/event/timed_event_scheduler.hpp>

#include <mutex>

void database_production::thread_update_alias_cache()
{
	while (true)
	{
		update_alias_cache();
		sleep_mili(std::chrono::hours(5));
	}
}

void database_production::update_alias_cache()
{
	// Get all alias informations
	auto _url_alias = decrypted_url + URL_GET_ALIAS;

	WPP_JVALUE args;
	args["token"] = decrypted_token;

	http_request::get()->do_request(_url_alias, std::bind(
		&database_production::handler_get_alias,
		this,
		std::placeholders::_1,
		std::placeholders::_2
	), args, true);
}

void database_production::thread_update_users_cache()
{
	while (true)
	{
		update_users_cache();
		sleep_mili(std::chrono::hours(12));
	}
}

void database_production::update_users_cache()
{
	// Get copy from users to check again your valiation
	mtx_users.lock();
	auto _users_copy = *users_cache;
	mtx_users.unlock();

	// If dont have any user yet.
	if (!_users_copy.size())
		return;

	for (auto& user : _users_copy)
		request_validate_user(user.second, false);

	// Put new list of users updated
	mtx_users.lock();
	*users_cache = _users_copy;
	mtx_users.unlock();
}

void database_production::handler_get_alias(
	uint32_t error,
	std::string & response
){
	WPP_JVALUE _jresponse = wpp::serialization::json::parse(response);
	std::map<uint32_t, alias_info> _aliasess;

	for (auto& res : _jresponse)
	{
		alias_info _new_alias(
			res["ip"].asString(),
			res["port_tcp"].asString(),
			res["port_udp"].asString()
		);
		_aliasess.insert({ std::atoi(res["id"].asString().data()), _new_alias });
	}

	mtx_alias.lock();
	*alias_cache = _aliasess;
	mtx_alias.unlock();

	cv.notify_all();
}

void database_production::run()
{
	// Decrypt configuration and initialize the variables with real value.
	initialize();

	boost::thread(boost::bind(&database_production::thread_update_alias_cache, this)).detach();
	boost::thread(boost::bind(&database_production::thread_update_users_cache, this)).detach();

	wait_alias_cache_updated();
}

void database_production::initialize()
{
	key_token = std::atoi(file_manager_bridge_base::singleton()->get("key_token").data());

	decrypted_url	= settings::decrypt_nphash(file_manager_bridge_base::singleton()->get_config_url(), key_token);
	decrypted_token = file_manager_bridge_base::singleton()->get_token();
}

void database_production::wait_alias_cache_updated()
{
	std::unique_lock<std::mutex> lock(mtx_alias);
	cv.wait(lock, [&] {
		return alias_cache->size();
	});
}

bool database_production::has_user_in_cache(const std::string & token)
{
	std::lock_guard<std::mutex> lock(mtx_users);
	return users_cache->find(token) != users_cache->end();
}

bool database_production::is_valid_user(const std::string & token)
{
	std::lock_guard<std::mutex> lock(mtx_users);
	return users_cache->operator[](token).is_valid;
}

void database_production::request_validate_user(user_info& _new_user, bool now)
{
	wpp::thread::event::event _wait;
	_wait.unset();

	auto _url = decrypted_url + "/validate_user";
	WPP_JVALUE args;
	args["token"] = _new_user.token;
	args["machine"] = _new_user.machine;
	args["sid"] = _new_user.sid;

	auto _lambda = [&](uint32_t code, std::string& res) {
		_new_user.is_valid = (res == "yes");
		_wait.set();
	};
	if(now)
		http_request::get()->do_request_now(_url, _lambda, args, true);
	else
		http_request::get()->do_request(_url, _lambda, args, true);
	// Wait request and added this user in cache
	_wait.wait();
}

alias_info database_production::get_alias_info(uint32_t alias_id)
{
	std::lock_guard<std::mutex> lock(mtx_alias);
	if (alias_cache->find(alias_id) != alias_cache->end()) {
		return alias_cache->operator[](alias_id);
	}
	return alias_info();
}

bool database_production::validate_user(
	char* token,
	char* machine_uid,
	uint32_t sid
){
	// TODO remove it
	return true;
	if (has_user_in_cache(token)) {
		return is_valid_user(token);
	}
	user_info _new_user(token, machine_uid, sid, false);
	request_validate_user(_new_user, true);

	// Check if need mutex here
	users_cache->insert(std::make_pair(_new_user.token, _new_user));

	return _new_user.is_valid;
}

database_production::database_production() :
	key_token(0)
{
	users_cache = std::make_shared<std::map<std::string, user_info>>();
	alias_cache = std::make_shared<std::map<uint32_t, alias_info>>();
}

