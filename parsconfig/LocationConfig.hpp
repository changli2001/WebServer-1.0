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
struct LocationBlockConfig {
	std::string 				LocationName; // access path for the location
	std::string 				BlockRootPath;
	bool 						autoindex;
	//  	bool                        IsBlockRoot;         /*check If the root Path is set inside The location Block*/
	//      bool                        IsBIndexed;             /*check if there is indexes files Directive*/
	//      bool                        IsMethodsSet;        /*check if methods are explicitly set for this location*/
	std::string 				upload_path;
	std::string 				cgi_extension;
	int 						return_code;
	std::string 				redirect_url;
	std::vector<std::string> 	Indexes;
	std::vector<std::string> 	allowedMethods;

	LocationBlockConfig() : autoindex(false), return_code(0) {}
};

#endif
