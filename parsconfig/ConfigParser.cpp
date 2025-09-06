#include "ConfigParser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <set>
#include <cstdlib>

ConfigParser::ConfigParser(const std::string &filename) : _filename(filename) {}

// Helper methods implementation

std::vector<ServerConfig> ConfigParser::parse()
{
	std::ifstream file(_filename.c_str());
	if (!file.is_open())
		throw std::runtime_error("Cannot open config file: " + _filename);

	std::vector<ServerConfig> servers;
	std::string line;
	int brace_depth = 0;
	bool in_server = false, in_location = false, expect_open_brace = false;

	ServerConfig currentServer;
	LocationConfig currentLoc;
	std::vector<std::string> locationPaths;

	// Server flags
	bool listen_flag = false, server_name_flag = false, max_body_flag = false, root_server_flag = false;

	// Location flags
	bool root_flag = false, index_flag = false, methods_flag = false, autoindex_flag = false;
	bool upload_flag = false, cgi_flag = false, return_flag = false;

	while (std::getline(file, line))
	{
		line.erase(std::remove(line.begin(), line.end(), ';'), line.end());
		if (line.empty() || line[0] == '#')
			continue;

		std::vector<std::string> tokens = tokenize(line);
		if (tokens.empty())
			continue;

		std::string key = tokens[0];

		if (key == "server")
		{
			if (in_server || in_location)
				throw std::runtime_error("Unexpected 'server' block inside another block");
			expect_open_brace = true;
		}
		else if (key == "location")
		{
			if (!in_server || in_location)
				throw std::runtime_error("Unexpected 'location' block");
			if (tokens.size() != 2 || tokens[1][0] != '/')
				throw std::runtime_error("Invalid location path: " + line);
			if (std::find(locationPaths.begin(), locationPaths.end(), tokens[1]) != locationPaths.end())
				throw std::runtime_error("Duplicate location path: " + tokens[1]);

			currentLoc = LocationConfig();
			currentLoc.path = tokens[1];
			locationPaths.push_back(currentLoc.path);
			expect_open_brace = true;
		}
		else if (key == "{")
		{
			if (!expect_open_brace)
				throw std::runtime_error("Unexpected '{'");
			expect_open_brace = false;
			if (!in_server)
				in_server = true;
			else
				in_location = true, root_flag = index_flag = methods_flag = autoindex_flag =
										upload_flag = cgi_flag = return_flag = false;
			++brace_depth;
		}
		else if (key == "}")
		{
			--brace_depth;
			if (in_location)
			{
				if (currentLoc.index_files.empty())
					currentLoc.index_files.push_back("index.html");
				if (currentLoc.methods.empty())
					currentLoc.methods.push_back("GET");
				if ((!currentLoc.upload_path.empty() &&
					 std::find(currentLoc.methods.begin(), currentLoc.methods.end(), "POST") == currentLoc.methods.end()) ||
					(!currentLoc.cgi_extension.empty() &&
					 std::find(currentLoc.methods.begin(), currentLoc.methods.end(), "POST") == currentLoc.methods.end()))
					throw std::runtime_error("upload_path or extenstion cgi requires POST method in: " + currentLoc.path);
				bool allows_post = std::find(currentLoc.methods.begin(), currentLoc.methods.end(), "POST") != currentLoc.methods.end();
				if (allows_post && currentLoc.upload_path.empty())
					throw std::runtime_error("Location '" + currentLoc.path + "' allows POST but has no upload_path");

				currentServer.locations.push_back(currentLoc);
				in_location = false;
			}
			else if (in_server && brace_depth == 0)
			{
				if (!listen_flag)
					throw std::runtime_error("Missing 'listen' directive");
				if (currentServer.locations.empty())
					throw std::runtime_error("Missing location blocks");
				bool hasRoot = false;
				for (size_t i = 0; i < currentServer.locations.size(); ++i)
					if (currentServer.locations[i].path == "/")
						hasRoot = true;
				for (size_t i = 0; i < currentServer.locations.size(); ++i)
				{
					if (currentServer.locations[i].root.empty())
					{
						currentServer.locations[i].root = currentServer.root;
					}
				}

				for (size_t i = 0; i < currentServer.locations.size(); ++i)
				{
					if (currentServer.locations[i].root.empty())
						currentServer.locations[i].root = currentServer.root;
				}
				if (return_flag && (root_flag || index_flag))
					throw std::runtime_error("Cannot use 'return' with 'root' or 'index' in the same location block: " + currentLoc.path);

				if (!hasRoot)
					throw std::runtime_error("Missing location '/' block");
				if (currentServer.server_name.empty())
					currentServer.server_name = "localhost";
				if (currentServer.ip.empty())
					currentServer.ip = "127.0.0.1";
				if (currentServer.port == 0)
					currentServer.port = 8080;
				if (currentServer.max_body_size == 0)
					currentServer.max_body_size = 1000000;
				std::ostringstream oss;
				oss << currentServer.ip << ":" << currentServer.port;
				std::string endpoint = oss.str();

				servers.push_back(currentServer);
				currentServer = ServerConfig();
				locationPaths.clear();
				in_server = false;
				listen_flag = server_name_flag = max_body_flag = false;
			}
		}
		else
		{
			if (!in_server)
				throw std::runtime_error("Directive outside of server block: " + key);

			if (in_location)
			{
				handleLocationDirective(tokens, currentLoc, root_flag, index_flag, methods_flag,
										autoindex_flag, upload_flag, cgi_flag, return_flag);
			}
			else
			{
				// REPLACED: The entire server directive handling with method call
				handleServerDirective(tokens, currentServer, listen_flag, server_name_flag,
									  max_body_flag, root_server_flag);
			}
		}
	}
	if (brace_depth != 0)
		throw std::runtime_error("Unclosed '{' block detected in config.");

	file.close();
	return servers;
}