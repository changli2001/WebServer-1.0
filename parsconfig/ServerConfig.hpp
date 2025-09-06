// ServerConfig.hpp
#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <map>
#include <vector>
#include "LocationConfig.hpp"

struct ServerConfig
{
	std::string ip;
	std::string root;
	int port;
	std::string server_name;
	size_t max_body_size;
	std::map<int, std::string> error_pages;
	std::vector<LocationConfig> locations;

	ServerConfig() : port(8888), max_body_size(1000000) {}
};
#endif
