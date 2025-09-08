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
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/fcntl.h>
    #include <sys/select.h>
    #include <sys/_select.h>
    #include <sys/_types/_fd_def.h>
    #include <cstdlib>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <errno.h>
    #include <vector>
    #include <set>
    
class   HttpServer {
    private:
        // ----- Data Members -----
        std::vector<ServerConfig>       Servers;               /*All The Servers listed in The configFile*/
        std::map<int, int>              SocketsPort;             /*each Fd with it specific port [fd : port]*/
        std::vector<int>                SocketFds;              /*Store the Fd of the created LIstening Sockets*/
        int                             enable;
        struct timeval                  timeout;                 /*The server time out */
        std::vector<int>                client_sockets;          /*A vector storing The clients Fds*/
        std::vector<Client*>            clients;                 /*A vector storing Client objects*/

        // ----- Server Methodes -----
        void        SetHintStruct(addrinfo  *hints);        /*Initiate a hint struct will be used be GetAddrinfo()*/
        int         enableSockReused(int SockFd);
        int         BindSock(int SockFd, addrinfo    *addr_strct);                             /*This Methode bind the socket at a specific port and address*/
        int         SetNonBlocking(int SockFd);
        void        AddToSet(fd_set  *MonitoredClients, int *maxFd);
        void        CheckListeningSocket(fd_set  *MonitoredClients, int *remaining_activity);
        int         CheckClients(fd_set  *MonitoredClients);
        void        NewClientConnected(int  ActiveFd);
        void        RemoveClient(int clientFd);  // Method to clean up disconnected clients
        void        CheckTimeouts();             // Method to check and remove timed-out clients
        bool        SetClientNonBlocking(int  fd);
        int         CreatSockets(int     PortNum);  /*This Methode Create The socket*/
        // ----- Getters ----
        // ----- Error Handling Methodes -----
        void        resoulvingFails(int status);            /*getaddrinfos fails*/
        // void        SocCreatFails(int PortNum);                        /*Fails To creat The socket*/
        void        SelectFails();
        void        acceptFails();
    public:
        HttpServer();
        HttpServer(const std::vector<ServerConfig>     _Servers);
        void        StartServer();                          /*From this function the server handlle clients*/
        void        InitializeServer();                     /*Creat all sockets In the configFile */
        ~HttpServer();
};

#endif