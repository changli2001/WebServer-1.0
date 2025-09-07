#include "../includes/HttpServer.hpp"
#include <iostream>
#include <netinet/in.h>
#include <sys/_types/_fd_def.h>
#include <sys/_types/_timeval.h>
#include <sys/socket.h>
#include <algorithm> 
#include <vector>

/*This Methode Make client socket to non-blocking mode*/
bool    HttpServer::SetClientNonBlocking(int  fd)
{
    
    if(fcntl(fd, F_SETFD, O_NONBLOCK) < 0)
    {
        std::cout << "Fails to set client socket to non-blocking "<< std::endl;
        std::cout << "Closing Connection .." << std::endl;
        close(fd);
        return (false);
    }
    return (true);
}


/*This Methode add all THe FDs to the monitoring set*/
void    HttpServer::AddToSet(fd_set  *MonitoredClients, int *maxFd)
{
    //Adding The passive socket
    for(std::vector<int>::const_iterator it = this->SocketFds.begin(); it != this->SocketFds.end(); it++)
    {
        int sockFd = *it;
        int port = this->SocketsPort[sockFd];
        FD_SET(sockFd, MonitoredClients);
        if (sockFd > *maxFd)
        {
            *maxFd = sockFd;
        }
        std::cout << YELLOW << "Server is Listening on Port [" << port <<  "]" << RESET << std::endl; 
    }
    for (size_t i = 0; i < this->client_sockets.size(); i++)
    {
        FD_SET(this->client_sockets[i], MonitoredClients);      //adding client to select set;
    }
}

/*Chck if a client is READABLE*/
int         HttpServer::CheckClients(fd_set  *MonitoredClients)
{
    std::cout << "cheking for clients " << std::endl;
    for(size_t i = 0; i < this->client_sockets.size(); i++)
    {
        int client_fd = client_sockets[i];
        if(FD_ISSET(client_fd, MonitoredClients))
        {
            return (client_fd);
        }
    }
    return (-1); // no client is READABLE
}


void    HttpServer::acceptFails()
{
    std::cerr << "Fails To accept The New CLient "<< std::endl;
}

void    HttpServer::SelectFails()
{
    std::cerr << RED << "Select Fails " << strerror(errno) << RESET << std::endl;
}



/*The function Take care of any new client connected to The server throught the passive 
    socket*/
void    HttpServer::NewClientConnected(int  ActiveFd)
{
    int newClient;
    
    newClient = accept(ActiveFd, NULL, NULL);
    if(newClient == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
                acceptFails();
        }
        return ;
    }
    std::cout << GREEN << "New Client connected To The server at port " << this->SocketsPort[ActiveFd] << RESET << std::endl;
    //Add The client to the list of Connected Clients;
    if (SetClientNonBlocking(newClient) == false)
        return ;
    this->client_sockets.push_back(newClient);
    const char* welcome = "Hello! You are connected to the simple server.\n";
    send(newClient, welcome, strlen(welcome), 0);
}



/*This Methode check if the socket_fd become READABLE*/
void     HttpServer::CheckListeningSocket(fd_set  *MonitoredClients, int *remaining_activity)
{
    for(std::vector<int>::const_iterator it = this->SocketFds.begin(); it != this->SocketFds.end() && *(remaining_activity) > 0
            ; ++it)
    {
        int SockFd = *it;
        if(FD_ISSET(SockFd, MonitoredClients))  // New connection detected;
        {
            (*remaining_activity)--;
            NewClientConnected(SockFd); //A client connect to the socket of FD = SockFd;
        }

    }
    return ;
}

/*The server run here*/
void    HttpServer::StartServer()
{
    int     S_status;           //To check The status of select()
    fd_set  MonitoredClients;   // The list to be monitored by Select()
    int     ready;
    int     MaxFds;
    int     remaining_activity;

    while(true)
    {
        MaxFds = 0;
        FD_ZERO(&MonitoredClients);
        AddToSet(&MonitoredClients, &MaxFds);
        S_status = select(MaxFds + 1, &MonitoredClients, NULL, NULL, NULL);
        if(S_status == -1)  //select fails
        {
            SelectFails();
            continue;
        }
        remaining_activity = S_status;
        CheckListeningSocket(&MonitoredClients, &remaining_activity);
        ready = CheckClients(&MonitoredClients);
    }
}