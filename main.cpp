#include "./includes/HttpServer.hpp"
#include "./includes/SrvConfig.hpp"
    #include <unistd.h>
    #include <iostream>

int main()
{
    SrvConfig    config;
    HttpServer   Server(&config);

    try
    {
        Server.BuildServer();
    }
    catch(std::exception &e){
        std::cerr << RED << e.what() << RESET << std::endl;
    }
    Server.StartServer();
}