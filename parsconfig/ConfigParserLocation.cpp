// Location block parsing
#include "ConfigParser.hpp"
#include <algorithm>
#include <cstdlib>
#include <stdexcept>

void ConfigParser::handleLocationDirective(const std::vector<std::string> &tokens,
										 LocationBlockConfig &currentLoc,
										 bool &root_flag,
										 bool &index_flag,
										 bool &methods_flag,
										 bool &autoindex_flag,
										 bool &upload_flag,
										 bool &cgi_flag,
										 bool &return_flag) {
	std::string key = tokens[0];
	std::string value = (tokens.size() > 1 ? tokens[1] : "");

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
		currentLoc.BlockRootPath = value;
		currentLoc.IsBlockRoot = true;

		root_flag = true;
	}
	else if (key == "index") {
		if (index_flag) throw std::runtime_error("Duplicate index in location");
		for (size_t i = 1; i < tokens.size(); ++i)
			currentLoc.Indexes.push_back(tokens[i]);
		currentLoc.IsBIndexed = true;
		index_flag = true;
	}
	else if (key == "autoindex") {
		if (autoindex_flag) throw std::runtime_error("Duplicate autoindex");
		currentLoc.autoindex = str_to_bool(value);
		currentLoc.IsAutoIndexSet = true;  // MARK AS EXPLICITLY SET
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
		currentLoc.allowedMethods = seen;
		currentLoc.IsMethodsSet = true;
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