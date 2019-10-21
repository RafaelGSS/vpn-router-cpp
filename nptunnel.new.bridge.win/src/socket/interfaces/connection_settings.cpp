#include "connection_settings.h"

#include <src/socket/packet_manipulation/intercall_methods.h>

#include <wpp/serialization/map_serialization.hpp>


void connection_settings::set_parameters(std::string param)
{
	parameters = wpp::serialization::map::deserialize_map(param);
}

bool connection_settings::is_valid_parameters()
{
	if (parameters.size() < 9)
		return false;

	if (
		parameters["ORIGINAL_DEST_IP"] == "" ||
		parameters["ORIGINAL_DEST_PORT"] == "" ||
		
		parameters["CONNECTIONTOKEN"] == "" ||
		parameters["USERTOKEN"] == "" ||

		parameters["VER"] == "" ||

		parameters["MACHINE"] == "" ||
		parameters["CONNECTIONID"] == "" ||

		parameters["ALIAS_ID"] == "" ||
		parameters["SID"] == ""
		) return false;

	return true;
}

bool connection_settings::is_valid_version(const std::string& version)
{
	auto _found = parameters.find("VER");

	if (_found != parameters.end()) {
		return _found->second == version;
	}
	return false;
}

bool connection_settings::has_hops()
{
	return get("HCT") != "";
}

uint32_t connection_settings::get_alias_id()
{
	if (_alias_id != -1)
		return _alias_id;
	
	try {
		_alias_id = std::stoi(get("ALIAS_ID"));
	}
	catch (std::exception&) {
		_alias_id = -1;
	}
	return _alias_id;
}

uint32_t connection_settings::get_current_hop_index()
{
	if (_current_hop_index != -1)
		return _current_hop_index;

	try {
		_current_hop_index = std::stoi(get("HI"));
	}
	catch (std::exception&) {
		LOGD("error on stoi HI");
		_current_hop_index = 0;
	}
	return _current_hop_index;
}

void connection_settings::update_route_index()
{
	parameters["HI"] = std::to_string(get_current_hop_index() + 1);
	_alias_id += 1;
}

std::shared_ptr<boost::asio::ip::tcp::endpoint> connection_settings::get_next_hop()
{
	if (!has_hops())
		return nullptr;

	auto jhops = intercall_methods::encrypted_packet_to_json(get("HCT"));
	auto next_index = get_current_hop_index() + 1;

	if (!jhops || !jhops.isArray() || !jhops.size() || jhops.size() <= next_index)
		return nullptr;

	std::vector<std::string> addresses;
	for (auto&& it : jhops)
	{
		if (!it["address"])
			return nullptr;
		addresses.push_back(it["address"].asString());
	}

	auto next_jhop = jhops[next_index];
	if (!next_jhop || !next_jhop["address"] || !next_jhop["tcp_port"])
		return nullptr;

	auto _address = next_jhop["address"].asString();
	auto _port    = next_jhop["tcp_port"].asString();

	return std::make_shared<boost::asio::ip::tcp::endpoint>(
				boost::asio::ip::address_v4::from_string(_address),
				std::stoi(_port)
			);

}

std::string connection_settings::get_parameters_to_string()
{
	return wpp::serialization::map::serialize_map(parameters);
}

std::string connection_settings::get(const std::string& key)
{
	auto _found = parameters.find(key);

	if (_found != parameters.end()) {
		return _found->second;
	}
	return std::string();
}

connection_settings::connection_settings() : 
	_alias_id(-1), _current_hop_index(-1)
{
}
