#pragma once
#include <string>
#include <functional>
#include <fstream>
#include <map>
#include <stdint.h>
#include <vector>
#include <ostream>
#include <iostream>
#include <chrono>
#include <mutex>

#include <libs/curl/curl.h>
#include <libs/curl/easy.h>
#pragma comment(lib, "libs/curl/libcurl.lib")


#include <wpp/serialization/json.hpp>

#define curl_lib CURL

class download_filter {
private:
	std::string content;
	std::ofstream* _out_file;

public:
	download_filter(std::ofstream* _file) {
		_out_file = _file;
	}

	download_filter() : _out_file(nullptr){}

	static size_t handle(char * data, size_t size, size_t nmemb, void * p) {
		return static_cast<download_filter*>(p)->handle_impl(data, size, nmemb);
	}

	size_t handle_impl(char * data, size_t size, size_t nmemb) {
		if (_out_file) {
			_out_file->write(data, size * nmemb);
		}
		else {
			content.append(data, size * nmemb);
		}
		return size * nmemb;
	}

	std::string get_content() {
		return content;
	}

	void release() {
		content = std::string();
	}
};

class base_request
{
protected:
	curl_lib *curl;
	CURLcode res;
	download_filter filter;

	void set_opt(
		const std::string _url,
		const uint32_t timeout
	);

	std::string merge_args_url(
		std::string url,
		WPP_JVALUE args
	);
public:
	base_request();

	virtual uint32_t get_request(
		std::string _url,
		std::string& out,
		uint32_t timeout = (uint32_t)-1
	);

	void release();

	virtual uint32_t post_request(
		std::string _url,
		std::string& out,
		WPP_JVALUE args = WPP_JVALUE(),
		uint32_t timeout = (uint32_t)-1
	);

	
};

