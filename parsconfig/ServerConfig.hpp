// ServerConfig.hpp
#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <map>
#include <vector>
#include "LocationConfig.hpp"

struct ServerConfig
{
	std::string 						Ip;
	std::string 						RootPath;
	int 								Port;			/*make it as a sting*/
 	// bool                            	IsRootSet;      
    // bool                            	IsAutoIndex;    
    // bool                            	IsIndexed;   
	std::string 						server_name;
	size_t 								MaxBodySize;
	std::map<int, std::string>		 	error_pages;
	std::vector<LocationBlockConfig> 		locations;
	// add The indexes 
	// ad auto index
	ServerConfig() : Port(8080), MaxBodySize(1000000) {}
};
#endif
