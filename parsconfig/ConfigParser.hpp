#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <vector>
#include <iostream>
#include "ServerConfig.hpp"

class ConfigParser {
	private:
		std::string _filename;

		std::vector<std::string> tokenize(const std::string &line);
		std::string normalize(std::string path);
		bool contains_dotdot(const std::string& path);
		bool str_to_bool(const std::string &str);
		bool isSimpleIPv4(const std::string &ip);
		bool isValidServerName(const std::string &name);
		bool isValidHttpMethod(const std::string &method);
		void handleLocationDirective(const std::vector<std::string> &tokens,
									LocationBlockConfig &currentLoc,
									bool &root_flag,
									bool &index_flag,
									bool &methods_flag,
									bool &autoindex_flag,
									bool &upload_flag,
									bool &cgi_flag,
									bool &return_flag);
	    void handleServerDirective(const std::vector<std::string> &tokens,
								ServerConfig &currentServer,
								bool &listen_flag,
								bool &server_name_flag,
								bool &max_body_flag,
								bool &root_server_flag);

	public:
		ConfigParser(const std::string &filename);
		std::vector<ServerConfig> parse();
};

#endif
