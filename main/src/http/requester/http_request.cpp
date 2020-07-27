#include "http_request.h"
#include <boost/thread.hpp>


void http_request::execute()
{
	while (true)
	{
		if (wait_next_task()) {
			auto _task = get_next_task();
			start_task(_task);
		}
	}
}

bool http_request::wait_next_task()
{
	std::unique_lock<std::mutex> lock(mtx);
	cv.wait(lock, [&] {
		return check_requests();
	});
	return true;
}

void http_request::do_request(
	std::string _url, 
	response_callback res, 
	WPP_JVALUE args,
	bool post, 
	uint32_t timeout,
	bool execute_in_thread
){
	std::lock_guard<std::mutex> lock(mtx);
	request_info req = { _url, res, args, timeout, post, execute_in_thread };
	requests.push_back(req);
	cv.notify_all();
}

bool http_request::check_requests()
{
	return requests.size() > 0;
}

void http_request::do_request_now(
	std::string _url,
	response_callback res,
	WPP_JVALUE args,
	bool post,
	uint32_t timeout,
	bool execute_in_thread
){
	std::lock_guard<std::mutex> lock(mtx);
	request_info req = { _url, res, args, timeout, post, execute_in_thread };
	requests.push_front(req);
	cv.notify_all();
}

request_info http_request::get_next_task()
{
	std::lock_guard<std::mutex> lock(mtx);
	
	request_info current_task = requests.front();
	requests.erase(requests.begin());
	return current_task;
}

void http_request::start_task(request_info & req)
{
	std::string retval;
	uint32_t _error = 0;
	if (req.post) {
		_error = post_request(req.url, retval, req.args, req.timeout);
	}
	else {
		_error = get_request(merge_args_url(req.url, req.args), retval, req.timeout);
	}
	if (_error != 200 && _error != 0) {
		retval = std::string();
	}

	if (req.execute_in_thread) {
		auto _cb = [&] {
			req.response(_error, retval);
		};
		boost::thread(_cb).detach();
		return;
	}

	req.response(_error, retval);
	sleep_mili(2000);
	
}

http_request::http_request()
{
	boost::thread(
		&http_request::execute,
		this
	).detach();
}

http_request * http_request::get()
{
	static http_request* singleton = nullptr;
	if (!singleton) {
		singleton = new http_request();
	}
	return singleton;
}
