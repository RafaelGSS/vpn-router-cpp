#include "tcp_connection.h"

#include <src/database/base/data_access.h>
#include <src/socket/tcp/tcp_monitor/tcp_monitor.h>
#include <config.h>

#include <wpp/net/ip/convert.hpp>
#include <wpp/type_stream/type_stream.hpp>

#include <boost/thread/lock_guard.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/thread/mutex.hpp>

void tcp_connection::process_to_alias()
{
	auto alias_id = config->get_alias_id();
	if (alias_id == (uint32_t)-1) {
		LOGE("alias undefined");
		close_connection();
		return;
	}

	auto info = data_access::get_default()->get_alias_info(alias_id);
	if (!info.ipv4_address) {
		LOGE("ipv4 on alias empty");
		close_connection();
		return;
	}

	connect_to_destination(wpp::net::ip::v4::convert::long_to_string(info.ipv4_address), info.tcp_port);
}

void tcp_connection::process_to_bridge(
	const std::shared_ptr<boost::asio::ip::tcp::endpoint>& next_endpoint
)
{
	if (config->get_current_hop_index() > 10) {
		close_connection();
		LOGD("current hop index invalid more than 10");
		return;
	}
	config->update_route_index();

	connect_to_destination(next_endpoint->address().to_string(), next_endpoint->port());
}

void tcp_connection::process_hops_connection()
{
	std::shared_ptr<boost::asio::ip::tcp::endpoint> next_endpoint = config->get_next_hop();
	if (!next_endpoint) {
		// Go to final
		//LOGI("packet to alias");
		process_to_alias();
	}
	else {
		// Go to next hop
		//LOGI("packet to bridge");
		process_to_bridge(next_endpoint);
	}
}

void tcp_connection::process_connection()
{
	/*if (config->get("reproxy") != "") {
		connect_to_destination("127.0.0.1", 100);
		return;
	}*/
	process_hops_connection();
}

void tcp_connection::send_status(
	const std::shared_ptr<boost::asio::ip::tcp::socket>& socket,
	const uint32_t status,
	bool close_con
){
	boost::lock_guard<boost::mutex> lock(_mtx);
	socket->async_write_some(
		boost::asio::buffer(
			&status,
			4
		),
		boost::bind(
			&tcp_connection::handler_send_status,
			this,
			this->shared_from_this(),
			boost::asio::placeholders::bytes_transferred,
			close_con
		)
	);
}

void tcp_connection::connect_to_destination(
	const std::string & address,
	const uint16_t & port
){
	auto _redir_socket  = reproxy_server;
	auto _client_socket = connection;

	try {
		if (_client_socket->local_endpoint().address().to_string() != "127.0.0.1") {
			_redir_socket->bind(boost::asio::ip::tcp::endpoint(_client_socket->local_endpoint().address(), 0));
		}
		_client_socket->set_option(boost::asio::ip::tcp::no_delay(true));
	}
	catch (std::exception& ec) {
		LOGD(ec.what());
	}

	// Check if trying to connec localhost
	/*if (address == "127.0.0.1") {
		LOGE("trying connect on localhost");
		close_connection();
		return;
	}*/

	auto next_destination = boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(address), port);

	_redir_socket->_sock->async_connect(
		next_destination,
		boost::bind(
			&tcp_connection::handler_connected_on_destination,
			this,
			this->shared_from_this(),
			boost::asio::placeholders::error
		)
	);

}

bool tcp_connection::is_closed()
{
	throw "Not implemented yet";
	return false;
}

void tcp_connection::close_connection()
{
	boost::lock_guard<boost::mutex> lock(_mtx);
	// Closing downstream
	//if (connection->is_open()) {
		boost::system::error_code ignore_ec;
		connection->cancel(ignore_ec);

		// Prevent exception
		ignore_ec.clear();
		connection->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignore_ec);
		
		ignore_ec.clear();
		connection->close(ignore_ec);
	//}
	
	// Closing upstream
	reproxy_server->close();
}

void tcp_connection::initialize_first_receive()
{
	//boost::lock_guard<boost::mutex> lock(_mtx);

	auto _memory_chunk = std::make_shared<memory_chunk>(DEFAULT_MEMORY_CHUNCK);

	connection->async_receive(
		boost::asio::buffer(_memory_chunk->get_data_ptr(), 2),
		boost::bind(
			&tcp_connection::handler_first_packet_size,
			this,
			this->shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred,
			_memory_chunk
		)
	);
}

tcp_connection::tcp_connection(
	const std::shared_ptr<boost::asio::ip::tcp::socket>& conn
)
 : connection(conn), reproxy_server(std::make_shared<tcp_reproxy_basic>(conn->get_io_service())),
	config(std::make_shared<connection_settings>())
{
	tcp_monitor::get()->new_user();
}

void tcp_connection::handler_first_packet_size(
	const std::shared_ptr<tcp_connection>& holder,
	const boost::system::error_code & ec,
	size_t bytes_transferred, 
	const std::shared_ptr<memory_chunk>& buff
){
	//boost::lock_guard<boost::mutex> lock(_mtx);
	
	uint32_t buffer_size = *(uint16_t*)buff->get_data_ptr();
	buff->set_total_size(buffer_size);

	// Checking if first packet is valid
	if (ec || bytes_transferred != 2 || buff->get_len() > DEFAULT_MEMORY_CHUNCK - 1) {
		LOGD(ec.message());
		close_connection();
		return;
	}

	boost::asio::async_read(
		*connection,
		boost::asio::buffer(
			buff->get_data_ptr(),
			buff->get_len()
		),
		boost::asio::transfer_all(),
		boost::bind(
			&tcp_connection::handler_first_packet,
			this,
			this->shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred,
			buff
		)
	);
	
}

void tcp_connection::handler_first_packet(
	const std::shared_ptr<tcp_connection>& holder, 
	const boost::system::error_code & ec,
	const size_t & bytes_transferred, 
	const std::shared_ptr<memory_chunk>& buff
){
	//boost::lock_guard<boost::mutex> lock(_mtx);

	if (ec || buff->get_len() != bytes_transferred) {
		LOGD(ec.message());
		close_connection();
		return;
	}

	std::string buffer(buff->get_data_ptr(), bytes_transferred);
	config->set_parameters(buffer);

	if (!config->is_valid_version(STUFF_VERSION)) {
		LOGE("version invalid");
		send_status(connection, connection_status::status_mismatch, true);
		return;
	}
	
	if (!config->is_valid_parameters()) {
		LOGD("parameters received is invalid on first packet");
		send_status(connection, connection_status::status_refused, true);
		return;
	}

	uint32_t sid = std::stoi(config->get("SID"));
	// Validate user
	if (!data_access::get_default()->validate_user(
		(char*)config->get("USERTOKEN").data(),
		(char*)config->get("MACHINE").data(),
		sid
	)) {
		LOGE("invalid user");
		send_status(connection, connection_status::status_refused, true);
		return;
	}

	process_connection();
}

void tcp_connection::handler_send_status(
	const std::shared_ptr<tcp_connection>& holder, 
	const size_t & bytes_transferred, 
	bool close_con
){
	if(close_con)
		close_connection();
}

void tcp_connection::handler_connected_on_destination(
	const std::shared_ptr<tcp_connection>& holder, 
	const boost::system::error_code& error
){
	reproxy_server->set_no_delay();
	if (error) {
		LOGD(error.message());
		close_connection();
		return;
	}
	auto _serialized_params = config->get_parameters_to_string();

	auto _buff = wpp::net::ip::socket::buffers::create_buffer_shared(_serialized_params.size()+2);
	
	// First add in two firstly bytes the length of string
	/*uint16_t _length = (uint16_t)_serialized_params.size();
	
	_buff->set_part_data(&_length, sizeof(uint16_t));
	_buff->set_part_data((void*)_serialized_params.c_str(), _serialized_params.size(), 2);*/

	auto _stream = std::make_shared<wpp::stream::type_stream>();
	_stream->add((uint16_t)_serialized_params.size());
	_stream->add_bytes((void*)_serialized_params.data(), _serialized_params.size());


	boost::asio::async_write(
		*(reproxy_server->_sock),
		boost::asio::buffer(
			_stream->get_buffer_ptr(),
			_stream->get_buffer_size()
		),
		boost::asio::transfer_all(),
		boost::bind(
			&tcp_connection::handler_send_header_on_destination,
			this,
			this->shared_from_this(),
			boost::asio::placeholders::bytes_transferred,
			boost::asio::placeholders::error,
			_buff
		)
	);
}

void tcp_connection::handler_send_header_on_destination(
	const std::shared_ptr<tcp_connection>& holder, 
	size_t bytes_transferred, 
	const boost::system::error_code& error,
	const std::shared_ptr<wpp::net::ip::socket::buffers::buffer_data>& buffer
){
	if (error || bytes_transferred <= 0) {
		LOGD(error.message());
		close_connection();
		return;
	}

	auto memory = std::make_shared<memory_chunk>(DEFAULT_MEMORY_CHUNCK);

	// Sizeof because only receive status from connection
	boost::asio::async_read(
		*(reproxy_server->_sock),
		boost::asio::buffer(memory->get_data_ptr(), sizeof(uint32_t)),
		boost::asio::transfer_all(),
		boost::bind(
			&tcp_connection::handler_recv_status_on_destination,
			this,
			this->shared_from_this(),
			boost::asio::placeholders::bytes_transferred,
			boost::asio::placeholders::error,
			memory
		)
	);
	

}

void tcp_connection::handler_recv_status_on_destination(
	const std::shared_ptr<tcp_connection>& holder,
	size_t bytes_transferred, 
	const boost::system::error_code & error,
	const std::shared_ptr<wpp::net::ip::socket::buffers::buffer_data>& buffer
){
	connection_status _state = (*(connection_status*)buffer->get_data_ptr());

	if (error || bytes_transferred != sizeof(uint32_t)) {
		LOGD(error.message());
		close_connection();
		return;
	}

	if (_state != connection_status::status_accepted) {
		LOGD("state different of accepted");
		send_status(connection, _state, true);
		return;
	}

	connection->async_send(
		boost::asio::buffer(
			buffer->get_data_ptr(),
			bytes_transferred
		),
		boost::bind(
			&tcp_connection::handler_send_ready_on_destination,
			this,
			this->shared_from_this(),
			boost::asio::placeholders::bytes_transferred,
			boost::asio::placeholders::error,
			buffer
		)
	);
}

void tcp_connection::handler_send_ready_on_destination(
	const std::shared_ptr<tcp_connection>& holder,
	size_t bytes_transferred,
	const boost::system::error_code & error, 
	const std::shared_ptr<wpp::net::ip::socket::buffers::buffer_data>& buffer
){
	if (error) {
		LOGD(error.message());
		close_connection();
		return;
	}

	set_listen_client();
	set_listen_server();
}


void tcp_connection::set_listen_client()
{
	auto buff = std::make_shared<memory_chunk>(DEFAULT_MEMORY_CHUNCK);

	connection->async_receive(
		boost::asio::buffer(
			buff->get_data_ptr(),
			buff->get_len()
		),
		boost::bind(
			&tcp_connection::handler_receive_client,
			this,
			this->shared_from_this(),
			boost::asio::placeholders::bytes_transferred,
			boost::asio::placeholders::error,
			buff
		)
	);
}

void tcp_connection::set_listen_server()
{
	auto buff = std::make_shared<memory_chunk>(DEFAULT_MEMORY_CHUNCK);

	reproxy_server->_sock->async_receive(
		boost::asio::buffer(
			buff->get_data_ptr(),
			buff->get_len()
		),
		boost::bind(
			&tcp_connection::handler_receive_server,
			this,
			this->shared_from_this(),
			boost::asio::placeholders::bytes_transferred,
			boost::asio::placeholders::error,
			buff
		)
	);
}

void tcp_connection::handler_receive_client(
	const std::shared_ptr<tcp_connection>& holder, 
	size_t bytes_transferred, 
	const boost::system::error_code & error,
	const std::shared_ptr<memory_chunk>& buffer
){
	if (error) {
		LOGD(error.message());
		close_connection();
		return;
	}
	
	boost::asio::async_write(
		*(reproxy_server->_sock),
		boost::asio::buffer(
			buffer->get_data_ptr(),
			bytes_transferred
		),
		boost::asio::transfer_all(),
		boost::bind(
			&tcp_connection::handler_write_server,
			this,
			this->shared_from_this(),
			boost::asio::placeholders::bytes_transferred,
			boost::asio::placeholders::error,
			buffer
		)
	);
}

void tcp_connection::handler_receive_server(
	const std::shared_ptr<tcp_connection>& holder,
	size_t bytes_transferred, 
	const boost::system::error_code & error, 
	const std::shared_ptr<memory_chunk>& buffer
){
	if (error) {
		LOGD(error.message());
		close_connection();
		return;
	}
	boost::asio::async_write(
		*connection,
		boost::asio::buffer(
			buffer->get_data_ptr(),
			bytes_transferred
		),
		boost::asio::transfer_all(),
		boost::bind(
			&tcp_connection::handler_write_client,
			this,
			this->shared_from_this(),
			boost::asio::placeholders::bytes_transferred,
			boost::asio::placeholders::error,
			buffer
		)
	);
}

void tcp_connection::handler_write_server(
	const std::shared_ptr<tcp_connection>& holder, 
	size_t bytes_transferred,
	const boost::system::error_code & error,
	const std::shared_ptr<memory_chunk>& buffer
){
	if (error) {
		LOGD(error.message());
		close_connection();
		return;
	}

	set_listen_client();
}

void tcp_connection::handler_write_client(
	const std::shared_ptr<tcp_connection>& holder,
	size_t bytes_transferred, 
	const boost::system::error_code & error,
	const std::shared_ptr<memory_chunk>& buffer
){
	if (error) {
		LOGD(error.message());
		close_connection();
		return;
	}
	set_listen_server();
}

tcp_connection::~tcp_connection()
{
	tcp_monitor::get()->close_user();
}
