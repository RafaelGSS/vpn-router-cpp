#ifndef CONNECTION_SETTINGS_H
#define CONNECTION_SETTINGS_H

#include <string>
#include <map>
#include <memory>

#include <boost/asio/ip/tcp.hpp>

// This class manager the parameters received on first packet, like hops, version, and all configurations to next hop
struct connection_settings {
	std::map<std::string, std::string> parameters;
	uint32_t _alias_id;
	uint32_t _current_hop_index;

	void set_parameters(std::string param);

	bool is_valid_parameters();
	bool is_valid_version(const std::string& version);
	bool has_hops();

	uint32_t get_alias_id();
	uint32_t get_current_hop_index();

	// Hop to next route
	void update_route_index();

	std::shared_ptr<boost::asio::ip::tcp::endpoint> get_next_hop();
	std::string get_parameters_to_string();

	std::string get(const std::string& key);

	connection_settings();
};

enum connection_status : uint32_t {
	status_bridge_closed,
	status_connecting,
	status_accepted,
	status_memory_full,
	status_refused,
	status_mismatch,
	status_server_max_users,
	status_max_client_connection
};

#endif