// LocationConfig.hpp
#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>

struct LocationConfig {
	std::string path; // access path for the location
	std::string root;
	std::vector<std::string> index_files;
	bool autoindex;
	std::vector<std::string> methods;
	std::string upload_path;
	std::string cgi_extension;
	int return_code;
	std::string redirect_url;

	LocationConfig() : autoindex(false), return_code(0) {}
};

#endif
