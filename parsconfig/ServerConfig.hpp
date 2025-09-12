#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <map>
#include <vector>
#include "LocationConfig.hpp"

struct ServerConfig
{
    std::string 						          Ip;
    std::string 						          RootPath;
    int                               Port;
    bool 								              IsRootSet;
    bool 								              IsAutoIndex;
    bool 								              IsIndexed;
    std::string 						          server_name;
    size_t 								            MaxBodySize;
    std::map<int, std::string> 			  error_pages;
    std::vector<LocationBlockConfig> 	locations;
    std::vector<std::string> 			    Indexes;      // Server-level index files
    bool 								              autoindex;                        // Server-level autoindex

    ServerConfig() 
        : Ip(""), 
          RootPath(""), 
          Port(8080), 
          IsRootSet(false), 
          IsAutoIndex(false), 
          IsIndexed(false), 
          server_name(""), 
          MaxBodySize(1000000), 
          error_pages(), 
          locations(), 
          Indexes(), 
          autoindex(false) {}
};
#endif