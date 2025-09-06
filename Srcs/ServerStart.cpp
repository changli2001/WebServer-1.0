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

void        HttpServer::Loading()
{
    std::cout << RED << "." << RESET << std::flush;
}

/*This Methode add all THe FDs to the monitoring set*/
void    HttpServer::AddToSet(fd_set  *MonitoredClients)
{
    //Adding The passive socket 
    FD_SET(this->SockFD, MonitoredClients);
    for (size_t i = 0; i < this->client_sockets.size(); i++)
    {
        FD_SET(this->client_sockets[i], MonitoredClients);
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

/*This Methode check if the socket_fd become READABLE*/
int     HttpServer::CheckListeningSocket(fd_set  *MonitoredClients)
{
    
    if(FD_ISSET(this->SockFD, MonitoredClients) != 0)
    {
        return (1);
    }
    return (0);
}


/*The function Take care of any new client connected to The server throught the passive 
    socket*/
void    HttpServer::NewClientConnected()
{
    int newClient;
    
    std::cout << "New client connect " << std::endl;
    newClient = accept(this->SockFD, NULL, NULL);
    if(newClient == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
                acceptFails();
        }
        return ;
    }
    this->SrvFDs++;
    std::cout << GREEN << "New Client connected To The server " << RESET << std::endl;
    //Add The client to the list of Connected Clients;
    if (SetClientNonBlocking(newClient) == false)
        return ;
    this->client_sockets.push_back(newClient);
    const char* welcome = "Hello! You are connected to the simple server.\n";
    send(newClient, welcome, strlen(welcome), 0);
}

void        HttpServer::HandlleClients(fd_set  *MonitoredClients, int fd)
{
    std::cout << "Closing The client Connection" << std::endl;
    //close The client fd
    close(fd);
    //rmouve it from the set
    FD_CLR(fd, MonitoredClients);
    //remouve The client from The list
    for(std::vector<int>::iterator it = this->client_sockets.begin(); it != this->client_sockets.end();)
    {
        if(*it == fd)
        {
            it = this->client_sockets.erase(it);
            break ;
        }
        else
            it++;
    }
}

/*The server run here*/
void    HttpServer::StartServer()
{
    int     S_status;           //To check The status of select()
    fd_set  MonitoredClients;   // The list to be monitored by Select()
    int     ready;

    while(true)
    {
        FD_ZERO(&MonitoredClients);
        AddToSet(&MonitoredClients);
        std::cout << YELLOW << "Server is Listening on [" 
        << this->serverConfiguration->Ip << "] : [" << this->ServerPort << "]"
            << RESET << std::endl;   
        S_status = select(this->SrvFDs + 1, &MonitoredClients, NULL, NULL, NULL);
        if(S_status == -1)  //select fails
        {
            SelectFails();
            continue;
        }
        if (CheckListeningSocket(&MonitoredClients) == 1)
        {
            NewClientConnected();
            continue;
        }
        ready = CheckClients(&MonitoredClients);
        if (ready != -1)
            HandlleClients(&MonitoredClients, ready);
    }
}