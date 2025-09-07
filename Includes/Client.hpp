#pragma once

    #include "./HttpServer.hpp"

struct SClientRequest
{
    std::string         	clientMethode;      /*WHta the client is requestinng from our server ?? GET {Other methodes coming soon}*/
    std::string         	clientSourceReq;    /*What the Hee the client wants from our server ??*/
    std::string         	httpVersion;        /*What the type of HTTP pro the client is using?*/
    std::string         	completURL;         /*The complet final URL that should be served to the client */
    
    char                	Request[4000];      //here were all the request will be stored;
    char                	ReqMethode[7];
    std::string				LocalisationName;
    char                	HttpVer[15];
};

class   Client{
    private:
        int     ClientFD;
        struct  Clien

    public:

}