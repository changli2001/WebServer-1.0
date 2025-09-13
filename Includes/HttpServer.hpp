#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

    #include <vector>
    #define BACKLOG             2
    #define TIMEOUT             0
    #define MAXSEC              1
    #define MAXUSEC             1
    #define READABLE            1
//                  --- includes ---
    #include "../parsconfig/ServerConfig.hpp"
    #include "colors.hpp"
    
// Forward declaration
class Client;
    #include <netdb.h>
    #include <cstring>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <iostream>
    #include <fstream>
    #include <sstream>
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/fcntl.h>
    #include <sys/select.h>
    // #include <sys/_select.h>
    // #include <sys/_types/_fd_def.h>
    #include <cstdlib>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <errno.h>
    #include <vector>
    #include <set>
    #include <map>
    
    #define ISREG 1
    #define ISDIRE 2
    #define NOTVALID -2
    #define NOTFOUND -1
class   HttpServer {
    private:
        // ----- Data Members -----
        std::vector<ServerConfig>       Servers;               /*All The Servers listed in The configFile*/
        std::map<int, int>              SocketsPort;             /*each Fd with it specific port [fd : port]*/
        std::vector<int>                SocketFds;              /*Store the Fd of the created LIstening Sockets*/
        int                             enable;
        struct timeval                  timeout;                 /*The server time out */
        std::map<int, Client*>          clients;                 /*Map storing Client objects by FD*/
        std::map<int, int>              clientToListeningSocket; /*Map client FD to listening socket FD*/

        // ----- Error Response Generation Members -----
        std::string                     _responseStartLine;      /*HTTP response start line*/
        std::string                     _responseHeaders;        /*HTTP response headers*/
        std::string                     _responseBody;           /*HTTP response body*/
        std::string                     finalResponse;           /*Complete HTTP response*/
        unsigned int                    status_nmbr;             /*HTTP status code*/
        std::string                     statusDescription;       /*HTTP status description*/
        ServerConfig*                   currentServerConfig;     /*Current server config for error pages*/

        // ----- Server Methodes -----
        void        SetHintStruct(addrinfo  *hints);        /*Initiate a hint struct will be used be GetAddrinfo()*/
        int         enableSockReused(int SockFd);
        int         BindSock(int SockFd, addrinfo    *addr_strct);                             /*This Methode bind the socket at a specific port and address*/
        int         SetNonBlocking(int SockFd);
        void        AddToSet(fd_set  *ReadableClients, fd_set *WritableClients, int *maxFd);
        void        CheckListeningSocket(fd_set  *MonitoredClients, int *remaining_activity);
        void        CheckReadableClients(fd_set  *MonitoredClients);
        void        CheckWriteableClients(fd_set  *MonitoredClients);

        void        NewClientConnected(int  ActiveFd);
        void        RemoveClient(int clientFd);  // Method to clean up disconnected clients
        void        CheckTimeouts();             // Method to check and remove timed-out clients
        bool        SetClientNonBlocking(int  fd);
        ServerConfig* getServerConfigByClientFD(int clientFd);  // Get ServerConfig for client FD
        ServerConfig* getServerConfigByPort(int port);          // Get ServerConfig by port
        int         CreatSockets(int     PortNum);  /*This Methode Create The socket*/
        // ----- Parsers ----
                //----- errors Response -----
        std::string generateErrorResponse(int ErrorCode);       /*Generate complete error response*/
        std::string generateErrorResponse(int ErrorCode, ServerConfig* serverConfig); /*Generate error response with specific config*/
        void        setCurrentServerConfig(ServerConfig* config); /*Set current server config for error generation*/
        std::string getStatusDes(unsigned int err);             /*Get error description by error number*/
        bool        isPageReadable() const;                     /*Check if error page path has read permissions*/
        bool        defaultErrPageProvided(const short Error) const; /*Check if default error page is provided*/
        void        genStartLine();                             /*Generate HTTP response start line*/
        void        genHeaders(const    std::string path);
                               /*Generate HTTP response headers*/
        void        genBody();                                  /*Generate HTTP response body*/
        // ----- Response -----
        
        std::string genreateResponse(Client *ClientObj, ServerConfig*   ServerConfig);
        
        
        
        
        
        
        
        //GET Methode
        std::string                     returnStatusPageHTML(unsigned short error) const; /*Return default HTML for error codes*/
        std::string                     HandleDirectories(const  std::string&    filePath, ServerConfig* currentServerConfig, const LocationBlockConfig* locationBlock);
        std::string                     HandleRegularFiles(const   std::string&    filePath);
        std::string                     ReturnHttpDefaultPage();
        bool                            isMethodAllowedForLocation(const LocationBlockConfig* locationBlock, std::string requestedMethod) const;
        std::string                     MachingLocationHandler(ServerConfig*                   currentServerConfig, Client* ClientObj);
        std::string                     GetMethodeExec(Client *ClientObj);
        int                             getPathStatus(const  std::string&    pathname);
        std::string                     autoIndexingServing(bool autoindex, const std::string& filePath);
        // Additional helper methods needed
        std::string                     handleIndexFiles(const std::vector<std::string>& indexFiles, const std::string& filePath);
        std::string                     redirection301(const std::string& path);
        std::string                     generateAutoindexPage(const  std::string&    filePath);
        std::string                     servRegFile(const    std::string&    filePath);










        // ----- Error Handling Methodes -----
        void        resoulvingFails(int status);            /*getaddrinfos fails*/
        // void        SocCreatFails(int PortNum);                        /*Fails To creat The socket*/
        void        SelectFails();
        void        acceptFails();
        
        //utils
        int         StringToInt(const std::string& str);    /*Convert string to int*/

    public:
        HttpServer();
        HttpServer(const std::vector<ServerConfig>     _Servers);
        void        StartServer();                          /*From this function the server handlle clients*/
        void        InitializeServer();                     /*Creat all sockets In the configFile */
        ~HttpServer();
};

#endif