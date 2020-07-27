#pragma once
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <utility>
#include <memory>

#include <boost/lexical_cast.hpp>

#include <config.h>

class file_manager
{
protected:
	std::map<std::string, std::string> fileParsed;
	std::mutex mtx;
public:
	template <typename T>
	T get_value(std::string&& key, T _default = T()) {
		try {
			std::lock_guard<std::mutex> lock(mtx);
			return boost::lexical_cast<T>(fileParsed[key]);
		}
		catch (...) {
			return _default;
		}
	}
	std::string get(std::string&& key) {
		std::lock_guard<std::mutex> lock(mtx);
		return fileParsed[key];
	}

	void set(std::string key, std::string value)
	{
		std::lock_guard<std::mutex> lock(mtx);
		this->fileParsed[key] = value;
	}

	file_manager(std::string fileName);
};
 

class file_manager_bridge_base : public file_manager {
public:
	file_manager_bridge_base(std::string fileName);

	virtual std::string get_config_url() = 0;

	virtual std::string get_bind_address() = 0;
	virtual std::string get_address() = 0;

	virtual std::string get_name() = 0;
	virtual int32_t get_core_afinnity_num() = 0;

	virtual uint16_t get_udp_port() = 0;
	virtual uint16_t get_tcp_port() = 0;

	virtual void set_token(std::string value) = 0;
	virtual std::string get_token() = 0;

	static file_manager_bridge_base* singleton(std::string fileName = SERVER_CONF_FILE);

};

class file_bridge_manager : public file_manager_bridge_base 
{
public:
	file_bridge_manager(std::string fileName);
	
	virtual std::string get_config_url();
	virtual std::string get_bind_address();

	virtual std::string get_address();

	virtual std::string get_name();
	virtual int32_t get_core_afinnity_num();

	virtual uint16_t get_udp_port();
	virtual uint16_t get_tcp_port();

	virtual void set_token(std::string value);
	virtual std::string get_token();
};

class file_bridge_manager_offline : public file_manager_bridge_base
{
public:
	file_bridge_manager_offline(std::string fileName);

	std::string get_config_url();
	std::string get_bind_address();

	std::string get_address();

	std::string get_name();
	int32_t get_core_afinnity_num();

	uint16_t get_udp_port();
	uint16_t get_tcp_port();

	virtual void set_token(std::string value);
	virtual std::string get_token();
};