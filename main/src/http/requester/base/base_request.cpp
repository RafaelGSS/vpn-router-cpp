#include "base_request.h"

base_request::base_request()
{
	res = CURLE_OK;
}

std::string base_request::merge_args_url(
	std::string url,
	WPP_JVALUE args
) {
	auto __args = args.asStringMap();
	std::string _url = url;
	for (auto& _arg : __args)
	{
		if (_url.find("?") == std::string::npos)
			_url += "?" + _arg.first + "=" + _arg.second;
		else
			_url += "&" + _arg.first + "=" + _arg.second;
	}
	return _url;
}

uint32_t base_request::get_request(
	std::string _url,
	std::string& out,
	uint32_t timeout
) {
	curl = curl_easy_init();

	if (!curl) {
		return 0;
	}

	set_opt(_url, timeout);
	CURLcode res = curl_easy_perform(curl);

	long http_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

	out = filter.get_content();

	if (res != CURLE_OK) {
		http_code = (uint32_t)-1;
	}
	release();
	return http_code;
}

void base_request::release()
{
	curl_easy_cleanup(curl);
	filter.release();
}

uint32_t base_request::post_request(
	std::string _url,
	std::string& out,
	WPP_JVALUE args,
	uint32_t timeout
) {
	curl = curl_easy_init();

	if (!curl) {
		return 0;
	}

	set_opt(_url, timeout);
	std::string content = "";
	for (auto& _vl : args.asStringMap())
	{
		if (!content.empty())
			content += "&";
		content += _vl.first + "=" + _vl.second;
	}
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content.data());

	CURLcode res = curl_easy_perform(curl);

	long http_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

	out = filter.get_content();

	if (res != CURLE_OK) {
		http_code = (uint32_t)-1;
	}
	release();
	return http_code;
}

void base_request::set_opt(
	const std::string _url,
	const uint32_t timeout
) {
	if (timeout != -1)
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)(timeout / 1000));
	curl_easy_setopt(curl, CURLOPT_URL, _url.data());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &filter);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &download_filter::handle);
}