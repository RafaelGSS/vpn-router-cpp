#pragma once
#include <sstream>
#include <map>
#include <string>

class settings
{
public:
	static std::map<std::string, std::string> load_file_config_parsed(std::string fileName);
	static std::stringstream& read_file(std::string&& fileName);

	static std::string decrypt_nphash(std::string value, int key = 7);
	static std::string encrypt_nphash(std::string value, int key = 7);

	settings();
};

