#ifndef SRVCONFIG_HPP
#define SRVCONFIG_HPP

    #include <vector>
    #include <map>
    #include <string> // Added missing include for std::string

//The locations block struct
struct LocationBlockConfig
{
    std::string                 path;                /*The name of The Location block Path */
    bool                        IsBlockRoot;         /*check If the root Path is set inside The location Block*/
    bool                        IsBAutoIndex;        /*check if The auto indexing Directive used in the lcoation*/
    bool                        IsBIndexed;             /*check if there is indexes files Directive*/
    bool                        IsMethodsSet;        /*check if methods are explicitly set for this location*/
    std::string                 BlockRootPath;           /*The root path inside The Location */
    std::vector<std::string>    allowedMethods;      /*The list of allowed HTTP methods for this location*/
    std::vector<std::string>    Indexes;             /*The list of index files In This Location*/
};


struct  SrvConfig {
    std::string                     Ip;             
    std::string                     Port;           
    bool                            IsRootSet;      
    bool                            IsAutoIndex;    
    bool                            IsIndexed;      
    std::string                     RootPath;       
    size_t                          MaxBodySize;    
    std::string                     server_name;    
    std::vector<std::string>        Indexes;        
    std::map<short, std::string>    error_pages;    
    std::vector<LocationBlockConfig>     locations;      

    SrvConfig() :   Ip("127.0.0.1"),
                    Port("9099"),
                    IsRootSet(true),
                    IsAutoIndex(false),
                    IsIndexed(false),
                    RootPath("/Users/ochangli/Desktop/code/"),
                    MaxBodySize(1000000),
                    server_name("WebSerbis")
                    {
                        Indexes.push_back("index.html");
                        Indexes.push_back("home.html");
                        Indexes.push_back("upload.html");
                        //error pages
                        error_pages[405] = "./errors/405.html";
                        error_pages[404] = "./errors/404.html";
                        error_pages[505] = "./errors/505.html";
                        error_pages[500] = "./errors/500.html";
                        error_pages[403] = "./errors/403.html";
                    // default location block ;
                        LocationBlockConfig     Block1;
                        Block1.path = "/";
                        Block1.IsBlockRoot  =   false;
                        Block1.IsBAutoIndex =   false;
                        Block1.IsBIndexed   =   false;
                        Block1.IsMethodsSet =   false;
                        Block1.BlockRootPath = "/Users/macbook/Desktop/websitetwo";
                        Block1.allowedMethods.push_back("GET");
                        Block1.allowedMethods.push_back("POST");
                        Block1.Indexes.push_back("index.html");
                    }
};

#endif