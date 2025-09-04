#include "../includes/HttpServer.hpp"

                                    // Constructors
HttpServer::HttpServer()
{
    this->enable = 1;
    this->ServerPort = "0";
    this->SockFD = 0;
    this->serverConfiguration = NULL;
    this->addr_strct = NULL;
}

HttpServer::HttpServer(const    SrvConfig*  configuration) : serverConfiguration(configuration)
{
    this->enable = 1;
    this->SockFD = 0;
    this->ServerPort = serverConfiguration->Port;
    this->addr_strct = NULL;
}

                                    // ---- Setter ----
void        HttpServer::SetSockFd(unsigned int _value)
{
    this->SockFD = _value;
}

void    HttpServer::SetHintStruct(addrinfo  *hints)
{
        std::memset(hints, 0, sizeof(*hints));
        hints->ai_family =      AF_INET;       // IPv4
        hints->ai_socktype =    SOCK_STREAM; // TCP stream sockets
        hints->ai_flags =       AI_PASSIVE;
}

                                    // ---- Getters ----

unsigned int HttpServer::getSrvSockFd()           const{return (this->SockFD);};
const std::string HttpServer::getSrvPortNmbr()          const{return (this->ServerPort);};
const SrvConfig*     HttpServer::getConfiguration()     const{return (this->serverConfiguration);};

                                    // ---- Errors Handller ----


void        HttpServer::FailToBind()
{ 
    close(this->SockFD);
    freeaddrinfo(this->addr_strct);
    throw std::runtime_error("fails to bind the socket ");
}

void        HttpServer::ListningFails()
{
    close(this->SockFD);
    std::cerr << "Listen fails" << strerror(errno) << std::endl;
}


void        HttpServer::SetsockoptFails()
{
    std::cerr << "Fail to Set socket option : " << strerror(errno) << std::endl;
    freeaddrinfo(this->addr_strct);
    close(this->SockFD);
}
/*This function handle The case when The socket creation fails*/
void        HttpServer::SocCreatFails()
{
    freeaddrinfo(this->addr_strct);
    throw std::runtime_error("Failed to create socket ");
}

/*This function handle The case when The getsockaddr  fails*/
void        HttpServer::resoulvingFails(int status)
{
    std::string errorMsg;

    errorMsg = "DNS resolution failed for [" + this->serverConfiguration->Ip + ":" + this->serverConfiguration->Port + "] : ";
    errorMsg += gai_strerror(status);
    throw std::runtime_error(errorMsg);    
}

                                        // ---- Methodes ---
void        HttpServer::enableSockReused()
{
    if (setsockopt(this->SockFD, SOL_SOCKET, SO_REUSEPORT, &(this->enable), sizeof(this->enable)) == -1)
    {
        SetsockoptFails();
    }
}

void    HttpServer::BindSock()
{
    if (bind(this->SockFD, this->addr_strct->ai_addr, this->addr_strct->ai_addr->sa_len)== -1)
    {
        FailToBind();
    }
    free(this->addr_strct);
    if ((listen(this->SockFD, BACKLOG)) != 0)
    {
        ListningFails();
    }
}
/*The function initiate a socket to use it in our server ...*/
void    HttpServer::CreatSocket()
{
    int    sockFd;
    int    status;
    struct addrinfo hints;

    SetHintStruct(&hints);
    
    status = getaddrinfo(this->serverConfiguration->Ip.c_str(), this->ServerPort.c_str(),
                            &hints, &(this->addr_strct));
    if (status != 0)
    {
        resoulvingFails(status);
    }
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockFd < 0)
    {
        SocCreatFails();
    }
    SetSockFd(sockFd);
    enableSockReused();
    BindSock();
}

                                        // ---- Destructor ----
HttpServer::~HttpServer(){};