#include "settings.h"

#include <config.h>

#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

#include <wpp/net/ip/socket/defines.h>

#include <boost/algorithm/string.hpp>


std::map<std::string, std::string> settings::load_file_config_parsed(std::string fileName)
{
	
	std::string line;
	std::map<std::string, std::string> retval;
	std::vector<std::string> str_splited;

	/*auto& string_stream
		= settings::read_file(std::move(fileName));*/
	std::fstream file;
	std::stringstream string_stream;

	file.open(fileName, std::fstream::in);
	if (!file) {
		LOGE("ERROR in open file");
		return std::map<std::string, std::string>();
	}

	string_stream << file.rdbuf();

	while (std::getline(string_stream, line)) {
		if (line[0] == '#') continue; // skip comments

		boost::split(str_splited, line, boost::is_any_of("="));
		boost::trim(str_splited[0]);
		boost::trim(str_splited[1]);
		retval[str_splited[0]] = str_splited[1];
	}
	return retval;
}

std::stringstream& settings::read_file(std::string&& fileName)
{
	std::fstream file;
	std::stringstream string_stream;

	file.open(fileName, std::fstream::in);
	if (!file) {
		LOGE("ERROR in open file");
		return string_stream;
	}
	string_stream << file.rdbuf();

	return string_stream;
}

std::string settings::decrypt_nphash(std::string value, int key)
{
	std::locale loc;

	std::string decrypted;
	for (unsigned char a : value)
	{
		uint32_t ascii = a;
		ascii = (key > 0) ? ascii - key : ascii + key;
		decrypted.push_back(ascii);
	}

	return decrypted;
}

std::string settings::encrypt_nphash(std::string value, int key)
{
	std::locale loc;
	std::transform(value.begin(), value.end(), value.begin(), ::toupper);

	std::string encrypted;
	for (unsigned char a : value)
	{
		uint16_t ascii = a;
		ascii = (key > 0) ? ascii + key : ascii - key;
		encrypted.push_back(ascii);
	}

	return encrypted;
}

settings::settings()
{
}

