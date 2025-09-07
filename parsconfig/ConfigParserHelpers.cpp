 // Helper methods

 #include "ConfigParser.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

std::vector<std::string> ConfigParser::tokenize(const std::string &line) {
	std::istringstream iss(line);
	std::vector<std::string> tokens;
	std::string token;
	while (iss >> token)
		tokens.push_back(token);
	return tokens;
}

std::string ConfigParser::normalize(std::string path) {
	while (path.length() > 1 && path[path.length() - 1] == '/')
		path.erase(path.length() - 1);
	return path;
}

bool ConfigParser::contains_dotdot(const std::string& path) {
	return path.find("..") != std::string::npos;
}

bool ConfigParser::str_to_bool(const std::string &str) {
	if (str == "on") return true;
	if (str == "off") return false;
	throw std::runtime_error("Invalid autoindex value: " + str);
}

bool ConfigParser::isSimpleIPv4(const std::string &ip) {
	int dotCount = 0;
	for (size_t i = 0; i < ip.length(); ++i) {
		char c = ip[i];
		if (c == '.') {
			if (i == 0 || i == ip.length() - 1 || ip[i - 1] == '.') return false;
			dotCount++;
		}
		else if (!isdigit(c))
			return false;
	}
	return dotCount == 3;
}

bool ConfigParser::isValidServerName(const std::string &name) {
	if (name == "localhost") return true;
	for (size_t i = 0; i < name.size(); ++i) {
		char c = name[i];
		if (!isalnum(c) && c != '.' && c != '-') return false;
	}
	return true;
}

bool ConfigParser::isValidHttpMethod(const std::string &method) {
	static const char* valid[] = { "GET", "POST", "DELETE" };
	for (size_t i = 0; i < 3; ++i)
		if (method == valid[i]) return true;
	return false;
}



size_t ConfigParser::parse_size_with_units(const std::string &size_str) {
    if (size_str.empty()) return 0;

    std::string str = size_str;
    size_t multiplier = 1;
    size_t number = 0;

    for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
        *it = std::toupper(*it);
    }

    size_t k_pos = str.find('K');
    size_t m_pos = str.find('M');
    size_t g_pos = str.find('G');

    if (g_pos != std::string::npos) {
        multiplier = 1024 * 1024 * 1024;
        str.erase(g_pos, 1);
    } else if (m_pos != std::string::npos) {
        multiplier = 1024 * 1024;
        str.erase(m_pos, 1);
    } else if (k_pos != std::string::npos) {
        multiplier = 1024;
        str.erase(k_pos, 1);
    }

    std::istringstream iss(str);
    if (!(iss >> number)) {
        throw std::runtime_error("Invalid size format: " + size_str);
    }

    return number * multiplier;
}