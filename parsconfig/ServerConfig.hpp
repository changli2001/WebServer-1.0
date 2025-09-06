#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <map>
#include <vector>
#include "LocationConfig.hpp"

struct ServerConfig
{
    std::string Ip;
    std::string RootPath;
    int Port;
    bool IsRootSet;
    bool IsAutoIndex;
    bool IsIndexed;
    std::string server_name;
    size_t MaxBodySize;
    std::map<int, std::string> error_pages;
    std::vector<LocationBlockConfig> locations;

    std::vector<std::string> Indexes;      // Server-level index files
    bool autoindex;                        // Server-level autoindex

    ServerConfig() : Port(8080), MaxBodySize(1000000), IsAutoIndex(false), IsIndexed(false), autoindex(false) {}
};
#endif