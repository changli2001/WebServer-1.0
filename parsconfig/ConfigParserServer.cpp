// Server block parsing

#include "ConfigParser.hpp"
#include <algorithm>
#include <cstdlib>
#include <stdexcept>

void ConfigParser::handleServerDirective(const std::vector<std::string> &tokens,
										 ServerConfig &currentServer,
										 bool &listen_flag,
										 bool &server_name_flag,
										 bool &max_body_flag,
										 bool &root_server_flag)
{
	std::string key = tokens[0];
	std::string value = (tokens.size() > 1 ? tokens[1] : "");

	if (key == "listen")
	{
		if (listen_flag)
			throw std::runtime_error("Duplicate listen directive");
		std::string::size_type pos = value.find(':');
		if (pos == std::string::npos || pos == 0 || pos == value.length() - 1)
			throw std::runtime_error("Invalid listen: " + value);
		std::string ip = value.substr(0, pos), portStr = value.substr(pos + 1);
		if (ip != "localhost" && !isSimpleIPv4(ip))
			throw std::runtime_error("Invalid IP: " + ip);
		int port = std::atoi(portStr.c_str());
		if (port < 0 || port > 65535)
			throw std::runtime_error("Port out of range: " + portStr);
		currentServer.ip = ip;
		currentServer.port = port;
		listen_flag = true;
	}
	else if (key == "root")
	{
		if (contains_dotdot(value))
			throw std::runtime_error("Invalid '..' in root path: " + value);
		if (root_server_flag)
			throw std::runtime_error("Duplicate root directive");
		if (value.empty() || value[0] != '/' || value.find("..") != std::string::npos)
			throw std::runtime_error("Invalid root: " + value);
		if (value[value.size() - 1] == '/')
			value = normalize(value);
		currentServer.root = value;
		root_server_flag = true;
	}
	else if (key == "server_name")
	{
		if (server_name_flag)
			throw std::runtime_error("Duplicate server_name");
		if (!isValidServerName(value))
			throw std::runtime_error("Invalid server_name: " + value);
		currentServer.server_name = value;
		server_name_flag = true;
	}
	else if (key == "client_max_body_size")
	{
		if (max_body_flag)
			throw std::runtime_error("Duplicate client_max_body_size");
		currentServer.max_body_size = std::atoi(value.c_str());
		if (currentServer.max_body_size <= 0)
			throw std::runtime_error("Invalid max_body_size");
		max_body_flag = true;
	}
	else if (key == "error_page")
	{
		if (tokens.size() != 3)
			throw std::runtime_error("Invalid error_page directive");
		if (contains_dotdot(value))
			throw std::runtime_error("Invalid '..' in root path: " + value);
		int code = std::atoi(tokens[1].c_str());
		if (code < 400 || code > 599)
			throw std::runtime_error("Invalid error code: " + tokens[1]);
		if (tokens[2][0] != '/' || tokens[2].find("..") != std::string::npos)
			throw std::runtime_error("Invalid error page path: " + tokens[2]);
		currentServer.error_pages[code] = tokens[2];
	}
	else
	{
		throw std::runtime_error("Unknown server directive: " + key);
	}
}