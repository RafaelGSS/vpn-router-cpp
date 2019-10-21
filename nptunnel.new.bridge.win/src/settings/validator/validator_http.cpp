#include "validator_http.h"

bool validator_http::is_valid_response(const std::string & response, uint32_t code)
{
	if (code != 200)
		return false;

	if (!response.size())
		return false;

	return true;
}

validator_http::validator_http()
{
}
