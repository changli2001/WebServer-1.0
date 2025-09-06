#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

    #include <vector>
    #define BACKLOG             2
    #define TIMEOUT             0
    #define MAXSEC              1
    #define MAXUSEC             1
    #define READABLE            1
//                  --- includes ---
    #include "SrvConfig.hpp"
    #include "colors.hpp"
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

class   HttpServer {
    private:
        // ----- Data Members -----
        std::string                 ServerPort;              /*The port That The Server is Listening ON*/
        unsigned int                SockFD;                  /*The File Descriptor Returned By The socket*/
        const       SrvConfig*      serverConfiguration;     /*This is a pointer To The Srver Configuration From The config file*/
        struct      addrinfo        *addr_strct;             /*The allocated struct by getaddrinfo() */
        int                         enable;
        int                         SrvFDs;                  /*All The fds incuded in the server (passive sock fd included)*/
        struct timeval              timeout;                 /*The server time out */
        std::vector<int>            client_sockets;          /*A vector storing The clients Fds*/
        // ----- Server Methodes -----
        void        SetHintStruct(addrinfo  *hints);                        /*Initiate a hint struct will be used be GetAddrinfo()*/
        void        SetSockFd(unsigned int _value);                            /*Assign The returend Fd to PortFD*/    
        void        enableSockReused();                     /*Make The Socket bind To a TIME_WAIT port */
        void        BindSock();                             /*This Methode bind the socket at a specific port and address*/
        void        SetNonBlocking();
        void        Loading();                              /*Called when Select() timeouted */
        void        AddToSet(fd_set  *MonitoredClients);
        int         CheckListeningSocket(fd_set  *MonitoredClients);
        int         CheckClients(fd_set  *MonitoredClients);
        void        NewClientConnected();
        void        HandlleClients(fd_set  *MonitoredClients, int fd);
        bool        SetClientNonBlocking(int  fd);

        // ----- Getters ----
        unsigned int                          getSrvSockFd()       const;
        const std::string                           getSrvPortNmbr()     const;
        const SrvConfig*                            getConfiguration()   const;
        // ----- Error Handling Methodes -----
        void        resoulvingFails(int status);                      /*getaddrinfos fails*/
        void        SocCreatFails();                        /*Fails To creat The socket*/
        void        SetsockoptFails();                      /*Fails To change the socket behavior*/
        void        ListningFails();                        /*Listen() API fails*/
        void        FailToBind();                           /*Bind() API  fails*/
        void        SelectFails();
        void        acceptFails();
    public:
        HttpServer();
        ~HttpServer();
        HttpServer(const    SrvConfig*  configuration);
        void        BuildServer();                          /*This Methode Create The socket*/
        void        StartServer();                          /*From this function the server handlle clients*/
};

#endif