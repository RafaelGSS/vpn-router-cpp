#pragma once
#include <string>
#include <stdint.h>

struct validator_http
{
public:
	static bool is_valid_response(const std::string& response, uint32_t code);
	validator_http();
};

