#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <map>
#include <exception>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <deque>

#include <src/http/requester/base/base_request.h>
#include <config.h>


using response_callback = std::function<void(uint32_t error, std::string& response)>;

struct request_info {
	request_info(
		std::string _url,
		response_callback res,
		WPP_JVALUE _args = WPP_JVALUE(),
		uint32_t _timeout = (uint32_t)-1,
		bool _post = false,
		bool _execute_in_thread = false
	) : 
	url(_url), response(res), args(_args), timeout(_timeout), post(_post), execute_in_thread(_execute_in_thread)
	{};

	std::string  url;
	response_callback response;
	WPP_JVALUE args;
	uint32_t timeout;
	bool post;
	bool execute_in_thread;
};

class http_request :
	public base_request
{
	std::condition_variable cv;
	std::mutex mtx;

	std::deque<request_info> requests;
	bool executing_task;

	void execute();
	bool wait_next_task();
	bool check_requests();
public:

	virtual void do_request(
		std::string _url,
		response_callback res,
		WPP_JVALUE args = WPP_JVALUE(),
		bool post = false,
		uint32_t timeout = (uint32_t)-1,
		bool execute_in_thread = false
	);


	virtual void do_request_now(
		std::string _url,
		response_callback res,
		WPP_JVALUE args = WPP_JVALUE(),
		bool post = false ,
		uint32_t timeout = (uint32_t)-1,
		bool execute_in_thread = false
	);

	request_info get_next_task();

	void start_task(request_info& req);

	http_request();

	static http_request* get();
};

#endif
