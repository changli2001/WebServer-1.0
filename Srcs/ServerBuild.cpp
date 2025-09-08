#include "../Includes/HttpServer.hpp"
#include <cstddef>
#include <cstdint>
#include <netdb.h>
#include <stdexcept>
#include <unistd.h>
#include <vector>


                                    // Constructors
HttpServer::HttpServer()
{
}

HttpServer::HttpServer(const std::vector<ServerConfig>     _Servers) : Servers(_Servers)
{
    this->enable = 1;
    this->timeout.tv_sec = MAXSEC;
    this->timeout.tv_usec = MAXUSEC;
}


void    HttpServer::SetHintStruct(addrinfo  *hints)
{
        std::memset(hints, 0, sizeof(*hints));
        hints->ai_family =      AF_INET;       // IPv4
        hints->ai_socktype =    SOCK_STREAM; // TCP stream sockets
        hints->ai_flags =       AI_PASSIVE;
}

/*This function handle The case when The getsockaddr  fails*/
void        HttpServer::resoulvingFails(int status)
{
    std::string errorMsg;
    errorMsg = "DNS resolution failed ";
    errorMsg += gai_strerror(status);
    std::cerr << RED << errorMsg << RESET << std::endl;
}

                                        // ---- Methodes ---
int        HttpServer::enableSockReused(int SockFd)
{
    if (setsockopt(SockFd, SOL_SOCKET, SO_REUSEPORT, &(this->enable), sizeof(this->enable)) == -1)
    {
        //all previous socket should be cleandup
        std::string erro =  "Fail to Set socket option : ";
        erro +=  strerror(errno) ;
        close(SockFd);
        std::cerr << RED << "Enable socket reused Fails .." << std::endl;
        return (-1);
    }
    return (0);
}

/*This Methode Make our socket to non-blocking mode*/
int    HttpServer::SetNonBlocking(int  SockFd)
{
    
    if(fcntl(SockFd, F_SETFL,O_NONBLOCK) < 0)
    {
        close(SockFd);
        std::cerr << RED << "changing socket flags fails" << std::endl;
        return (-1);
    }
    return (0);
}

int    HttpServer::BindSock(int SockFd, addrinfo    *addr_strct)
{
    if (bind(SockFd, addr_strct->ai_addr, addr_strct->ai_addrlen) == -1)
    {
        close(SockFd);
        std::cerr << RED << "fails to bind the socket " << std::endl;
        return (-1);
    }
    freeaddrinfo(addr_strct);
    if ((listen(SockFd, BACKLOG)) != 0)
    {
        close(SockFd);
        std::cerr << "Listen fails" << strerror(errno) << std::endl;
        return (-1);
    }
    return (0);
}


/*The function initiate a socket to use it in our server ...*/
int    HttpServer::CreatSockets(int     PortNum)
{
    int                     sockFd;
    int                     status;
    struct addrinfo         hints;
    struct      addrinfo    *addr_strct;

    SetHintStruct(&hints);
    
    status = getaddrinfo(NULL, std::to_string(PortNum).c_str(), &hints, &addr_strct); //c++98 ??
    if (status != 0)
    {
        resoulvingFails(status);
        return (-1);
    }
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockFd < 0)
    {
        freeaddrinfo(addr_strct);
        return (-1);
    }
    if (SetNonBlocking(sockFd) == -1) {freeaddrinfo(addr_strct); return (-1);}
    if (enableSockReused(sockFd) == -1) {freeaddrinfo(addr_strct);return (-1);}
    if (BindSock(sockFd, addr_strct ) == -1) {freeaddrinfo(addr_strct);return (-1);}
    return (sockFd);
}

/*the function creat sockets for all servers listed in The configFile*/
void        HttpServer::InitializeServer()
{
    std::set<int>               UsedPorts;
    std::vector<ServerConfig>   Srv = this->Servers;
    int                         SocketFd;

    for(std::vector<ServerConfig>:: const_iterator  it = Srv.begin(); it != Srv.end(); ++it)
    {
        const ServerConfig& tmp = *it;
        int Port = tmp.Port;
        if(UsedPorts.find(Port) == UsedPorts.end())
        {
            UsedPorts.insert(Port);
            SocketFd = CreatSockets(Port);
            if(SocketFd == -1)
            {
                throw std::runtime_error("Fail To creat the socket");
            }
            else {
                this->SocketFds.push_back(SocketFd);        //Adding The returned Fd of The socket to the list
                this->SocketsPort[SocketFd] = tmp.Port;
            }
        }
        else
        {
            std::cout << RED << "Port Already Used" << RESET << std::endl;
        }
    }
}
                                        // ---- Destructor ----
HttpServer::~HttpServer(){};