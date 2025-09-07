// LocationConfig.hpp
#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>

// struct LocationBlockConfig {
// 	std::string 				BlokName; // access path for the location
// 	std::string 				root;
// 	std::vector<std::string> 	index_files;
// 	bool 						autoindex;
// 	std::vector<std::string> 	methods;
// 	std::string 				upload_path;
// 	std::string 				cgi_extension;
// 	int 						return_code;
// 	std::string 				redirect_url;

// 	LocationBlockConfig() : autoindex(false), return_code(0) {}
// };
struct LocationBlockConfig 
{
    std::string                     LocationName;
    std::string                     BlockRootPath;
    bool                            autoindex;
    bool                            IsBlockRoot;
    bool                            IsBIndexed;
    bool                            IsMethodsSet;
    bool                            IsAutoIndexSet;  // ADD THIS
    std::string                     upload_path;
    std::string                     cgi_extension;
    int                             return_code;
    std::string                     redirect_url;
    std::vector<std::string>        Indexes;
    std::vector<std::string>        allowedMethods;

    LocationBlockConfig() : LocationName(""), BlockRootPath(""), autoindex(false), 
                            IsBlockRoot(false), IsBIndexed(false), IsMethodsSet(false), 
                            IsAutoIndexSet(false), upload_path(""), cgi_extension(""), 
                            return_code(0), redirect_url("") // INITIALIZE
                            {}
};


#endif
