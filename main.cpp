#include "./includes/HttpServer.hpp"
#include "./includes/SrvConfig.hpp"
#include "./parsconfig/ConfigParser.hpp"

    #include <unistd.h>
    #include <iostream>

int main(int argc, char **argv)
{
    SrvConfig       config;
    HttpServer      Server(&config);
    std::string     configFile;

	if (argc > 2)
	{
		std::cerr << "No valide number of Args";
		return 1;
	}
    if(argc == 1){
        configFile = "./config/default.conf";
    }
    else {
        configFile = argv[1];
    }
    try
    {
        ConfigParser parser(configFile); // creating object for parsing
        std::vector<ServerConfig> configs = parser.parse(); // start parsing
    }
	catch (const std::exception &e)
	{
		std::cerr << "Config parsing error: " << e.what() << std::endl;
		return 1;
	}
    try
    {
        Server.BuildServer();
    } 
    catch(std::exception &e){
        std::cerr << RED << e.what() << RESET << std::endl;
    }
    Server.StartServer();
}