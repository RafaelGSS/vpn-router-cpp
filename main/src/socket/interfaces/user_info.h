#pragma once
#include <string>
#include <stdint.h>

struct user_info
{
	std::string token;
	std::string machine;
	uint32_t sid;

	bool is_valid;

	user_info(std::string _token, std::string _machine, uint32_t _sid, bool valid) :
		token(_token), machine(_machine), sid(_sid), is_valid(valid) {}
	user_info() {}
};