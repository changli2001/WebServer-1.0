#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <set>
#include <cstdlib>
#include "ServerConfig.hpp"

class ConfigParser {
	std::string _filename;

	std::vector<std::string> tokenize(const std::string &line) {
		std::istringstream iss(line);
		std::vector<std::string> tokens;
		std::string token;
		while (iss >> token)
			tokens.push_back(token);
		return tokens;
	}
	std::string normalize(std::string path) {
		// Remove trailing slashes (but keep root '/')
		while (path.length() > 1 && path[path.length() - 1] == '/')
			path.erase(path.length() - 1);
		return path;
	}
	bool contains_dotdot(const std::string& path) {
		return path.find("..") != std::string::npos;
	}
	bool str_to_bool(const std::string &str) {
		if (str == "on") return true;
		if (str == "off") return false;
		throw std::runtime_error("Invalid autoindex value: " + str);
	}

	bool isSimpleIPv4(const std::string &ip) {
		int dotCount = 0;
		for (size_t i = 0; i < ip.length(); ++i) {
			char c = ip[i];
			if (c == '.') {
				if (i == 0 || i == ip.length() - 1 || ip[i - 1] == '.') return false;
				dotCount++;
			} else if (!isdigit(c))
				return false;
		}
		return dotCount == 3;
	}

	bool isValidServerName(const std::string &name) {
		if (name == "localhost") return true;
		for (size_t i = 0; i < name.size(); ++i) {
			char c = name[i];
			if (!isalnum(c) && c != '.' && c != '-') return false;
		}
		return true;
	}

	bool isValidHttpMethod(const std::string &method) {
		static const char* valid[] = { "GET", "POST", "DELETE" };
		for (size_t i = 0; i < 3; ++i)
			if (method == valid[i]) return true;
		return false;
	}

public:
	ConfigParser(const std::string &filename) : _filename(filename) {}

	std::vector<ServerConfig> parse() {
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
		bool listen_flag = false, server_name_flag = false, max_body_flag = false , root_server_flag = false;

		// Location flags
		bool root_flag = false, index_flag = false, methods_flag = false, autoindex_flag = false;
		bool upload_flag = false, cgi_flag = false, return_flag = false;

		while (std::getline(file, line)) {
			line.erase(std::remove(line.begin(), line.end(), ';'), line.end());
			if (line.empty() || line[0] == '#') continue;

			std::vector<std::string> tokens = tokenize(line);
			if (tokens.empty()) continue;

			std::string key = tokens[0];

			if (key == "server") {
				if (in_server || in_location)
					throw std::runtime_error("Unexpected 'server' block inside another block");
				expect_open_brace = true;
			}
			else if (key == "location") {
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
			else if (key == "{") {
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
			else if (key == "}") {
				--brace_depth;
				if (in_location) {
					if (currentLoc.index_files.empty())
						currentLoc.index_files.push_back("index.html");
					if (currentLoc.methods.empty())
						currentLoc.methods.push_back("GET");
					if ((!currentLoc.upload_path.empty() &&
						std::find(currentLoc.methods.begin(), currentLoc.methods.end(), "POST") == currentLoc.methods.end()) || (!currentLoc.cgi_extension.empty() &&
						std::find(currentLoc.methods.begin(), currentLoc.methods.end(), "POST") == currentLoc.methods.end()))
						throw std::runtime_error("upload_path or extenstion cgi requires POST method in: " + currentLoc.path);
					bool allows_post = std::find(currentLoc.methods.begin(), currentLoc.methods.end(), "POST") != currentLoc.methods.end();
					if (allows_post && currentLoc.upload_path.empty())
						throw std::runtime_error("Location '" + currentLoc.path + "' allows POST but has no upload_path");


					currentServer.locations.push_back(currentLoc);
					in_location = false;
				}
				else if (in_server && brace_depth == 0) {
					if (!listen_flag)
						throw std::runtime_error("Missing 'listen' directive");
					if (currentServer.locations.empty())
						throw std::runtime_error("Missing location blocks");
					bool hasRoot = false;
					for (size_t i = 0; i < currentServer.locations.size(); ++i)
						if (currentServer.locations[i].path == "/") hasRoot = true;
					for (size_t i = 0; i < currentServer.locations.size(); ++i) {
						if (currentServer.locations[i].root.empty()) {
							// if (currentServer.root.empty()) {
							// 	throw std::runtime_error(
							// 		"Missing root in location '" + currentServer.locations[i].path +
							// 		"' and no server-level root to inherit from.");
							// }
							currentServer.locations[i].root = currentServer.root;
						}
					}

					// std::set<std::string> usedRoots;
					// for (size_t i = 0; i < currentServer.locations.size(); ++i) {
					// 	std::string root = currentServer.locations[i].root;
					// 	if (!root.empty()) {
					// 		if (!usedRoots.insert(root).second)
					// 			throw std::runtime_error("Duplicate root value detected: " + root);
					// 	}
					// }
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
			else {
				if (!in_server)
					throw std::runtime_error("Directive outside of server block: " + key);

				std::string value = (tokens.size() > 1 ? tokens[1] : "");

				if (in_location) {
					if (key == "root") {
						if (contains_dotdot(value))
							throw std::runtime_error("Invalid '..' in root path: " + value);
						if (value == "/")
							throw std::runtime_error("Root cannot be set for the root location");
						if (value[value.size() - 1] == '/')
							value = normalize(value);
						if (root_flag) throw std::runtime_error("Duplicate root in location");
						if (value[0] == '"' && value[value.size() - 1] == '"')
							value = value.substr(1, value.size() - 2);
						if (value.empty() || value[0] != '/' || value.find("..") != std::string::npos)
							throw std::runtime_error("Invalid root: " + value);
						// if inside quotes we tream it it as a path
						currentLoc.root = value;
						root_flag = true;
					}
					else if (key == "index") {
						if (index_flag) throw std::runtime_error("Duplicate index in location");
						for (size_t i = 1; i < tokens.size(); ++i)
							currentLoc.index_files.push_back(tokens[i]);
						index_flag = true;
					}
					else if (key == "autoindex") {
						if (autoindex_flag) throw std::runtime_error("Duplicate autoindex");
						currentLoc.autoindex = str_to_bool(value);
						autoindex_flag = true;
					}
					else if (key == "methods") {
						std::vector<std::string> seen;
						for (size_t i = 1; i < tokens.size(); ++i) {
							if (!isValidHttpMethod(tokens[i]))
								throw std::runtime_error("Invalid method: " + tokens[i]);
							if (std::find(seen.begin(), seen.end(), tokens[i]) == seen.end())
								seen.push_back(tokens[i]);
						}
						currentLoc.methods = seen;
						methods_flag = true;
					}
					else if (key == "upload_path") {
						if (upload_flag) throw std::runtime_error("Duplicate upload_path");
						if (value.empty() || value[0] != '/' || value.find("..") != std::string::npos)
							throw std::runtime_error("Invalid upload_path: " + value);
						if (contains_dotdot(value))
							throw std::runtime_error("Invalid '..' in root path: " + value);
						currentLoc.upload_path = value;
						upload_flag = true;
					}
					else if (key == "cgi_extension") {
						if (value.empty() || value[0] != '.' || value.find("..") != std::string::npos || value.find('/') != std::string::npos)
							throw std::runtime_error("Invalid cgi_extension: " + value);
						if (cgi_flag) throw std::runtime_error("Duplicate cgi_extension");
						currentLoc.cgi_extension = value;
						cgi_flag = true;
					}
					else if (key == "return") {
						if (return_flag || tokens.size() != 3)
							throw std::runtime_error("Invalid or duplicate return");
						currentLoc.return_code = std::atoi(tokens[1].c_str());
						currentLoc.redirect_url = tokens[2];
						if (currentLoc.return_code < 300 || currentLoc.return_code > 399)
							throw std::runtime_error("Invalid redirect code: " + tokens[1]);
						return_flag = true;
					}
					else {
						throw std::runtime_error("Unknown directive in location: " + key);
					}
				}
				else {
					if (key == "listen") {
						if (listen_flag) throw std::runtime_error("Duplicate listen directive");
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
					else if (key == "root") {
						if (contains_dotdot(value))
							throw std::runtime_error("Invalid '..' in root path: " + value);
						if (root_server_flag) throw std::runtime_error("Duplicate root directive");
						if (value.empty() || value[0] != '/' || value.find("..") != std::string::npos)
							throw std::runtime_error("Invalid root: " + value);
						if (value[value.size() - 1] == '/')
							value = normalize(value);
						currentServer.root = value;
						root_server_flag = true;
					}
					else if (key == "server_name") {
						if (server_name_flag) throw std::runtime_error("Duplicate server_name");
						if (!isValidServerName(value))
							throw std::runtime_error("Invalid server_name: " + value);
						currentServer.server_name = value;
						server_name_flag = true;
					}
					else if (key == "client_max_body_size") {
						if (max_body_flag) throw std::runtime_error("Duplicate client_max_body_size");
						currentServer.max_body_size = std::atoi(value.c_str());
						if (currentServer.max_body_size <= 0)
							throw std::runtime_error("Invalid max_body_size");
						max_body_flag = true;
					}
					else if (key == "error_page") {
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
						throw std::runtime_error("Unknown server directive: " + key);
				}
			}
		}
		if (brace_depth != 0)
			throw std::runtime_error("Unclosed '{' block detected in config.");

		file.close();
		return servers;
	}
};

#endif
