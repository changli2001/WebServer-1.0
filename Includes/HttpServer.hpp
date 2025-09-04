#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

    #define BACKLOG             2


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


class   HttpServer {
    private:
        // ----- Data Members -----
        std::string                 ServerPort;              /*The port That The Server is Listening ON*/
        unsigned int                SockFD;                  /*The File Descriptor Returned By The socket*/
        const       SrvConfig*      serverConfiguration;     /*This is a pointer To The Srver Configuration From The config file*/
        struct      addrinfo        *addr_strct;             /*The allocated struct by getaddrinfo() */
        int                         enable;
        // ----- Server Methodes -----
        void        SetHintStruct(addrinfo  *hints);                        /*Initiate a hint struct will be used be GetAddrinfo()*/
        void        SetSockFd(unsigned int _value);                            /*Assign The returend Fd to PortFD*/    
        void        enableSockReused();                     /*Make The Socket bind To a TIME_WAIT port */
        void        BindSock();                             /*This Methode bind the socket at a specific port and address*/
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
    public:
        HttpServer();
        ~HttpServer();
        HttpServer(const    SrvConfig*  configuration);
        void        CreatSocket();                          /*This Methode Create The socket*/

};

#endif