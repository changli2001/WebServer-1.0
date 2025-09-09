#include "../Includes/HttpServer.hpp"
#include "../Includes/Client.hpp"
#include <iostream>
#include <netinet/in.h>
#include <sys/_types/_fd_def.h>
#include <sys/_types/_timeval.h>
#include <sys/socket.h>
#include <algorithm> 
#include <vector>


void    HttpServer::acceptFails()
{
    std::cerr << "Fails To accept The New CLient "<< std::endl;
}

void    HttpServer::SelectFails()
{
    std::cerr << RED << "Select Fails " << strerror(errno) << RESET << std::endl;
}

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
void    HttpServer::AddToSet(fd_set  *ReadableClients, fd_set *WritableClients, int *maxFd)
{
    //Adding The passive socket
    for(std::vector<int>::const_iterator it = this->SocketFds.begin(); it != this->SocketFds.end(); it++)
    {
        int sockFd = *it;
        int port = this->SocketsPort[sockFd];
        FD_SET(sockFd, ReadableClients);
        if (sockFd > *maxFd)
        {
            *maxFd = sockFd;
        }
        std::cout << YELLOW << "Server is Listening on Port [" << port <<  "]" << RESET << std::endl; 
    }
    //Add clients based on their state
    for (size_t i = 0; i < this->clients.size(); i++)
    {
        Client* clientObj = clients[i];
        int client_fd = clientObj->getFD();
        if (clientObj->getState() == READSTATE)
        {
            FD_SET(client_fd, ReadableClients);
        }
        if (clientObj->getState() == WRITESTATE)
        {
            FD_SET(client_fd, WritableClients);
        }
        if (client_fd > *maxFd)
        {
            *maxFd = client_fd;
        }
    }
}

//...
/*Check if a client is READABLE and handle the data*/
int         HttpServer::CheckReadableClients(fd_set  *MonitoredClients)
{
    for(size_t i = 0; i < this->client_sockets.size(); i++)
    {
        int client_fd = client_sockets[i];
        if(FD_ISSET(client_fd, MonitoredClients))
        {
            Client* clientObj = NULL;
            for (size_t j = 0; j < clients.size(); j++)
            {
                if (clients[j]->getFD() == client_fd) {
                    clientObj = clients[j];
                    break;
                }
            }
            if (clientObj != NULL && clientObj->getState() == READSTATE)
            {
                if (!clientObj->handleEchoRead())
                {
                    RemoveClient(client_fd);
                    return (-1);  // Client was removed
                }
                else
                {
                    clientObj->setState(WRITESTATE);
                }
            }
            return (client_fd);
        }
    }
    return (-1);
}

int         HttpServer::CheckWriteableClients(fd_set  *MonitoredClients)
{
    for(size_t i = 0; i < this->client_sockets.size(); i++)
    {
        int client_fd = client_sockets[i];
        if(FD_ISSET(client_fd, MonitoredClients))
        {
            Client* clientObj = NULL;
            for (size_t j = 0; j < clients.size(); j++)
            {
                if (clients[j]->getFD() == client_fd) {
                    clientObj = clients[j];
                    break;
                }
            }
            if (clientObj != NULL && clientObj->getState() == WRITESTATE)
            {
                if (!clientObj->handleEchoWrite())
                {
                    std::cout << "Client " << client_fd << " echo write failed, disconnecting" << std::endl;
                    RemoveClient(client_fd);
                    return (-1);
                }
                else
                {
                    clientObj->setState(READSTATE);
                    std::cout << "Client " << client_fd << " echo sent, back to READSTATE" << std::endl;
                }
            }
            return (client_fd);
        }
    }
    return (-1);
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
    if (SetClientNonBlocking(newClient) == false)
        return ;
    Client* clientObj = new Client(newClient);
    this->client_sockets.push_back(newClient);
    this->clients.push_back(clientObj);  // Store the client object
    clientObj->updateActivity();        // Update activity after sending welcome message
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

/*Remove a client and clean up its resources*/
void    HttpServer::RemoveClient(int clientFd)
{
    // Remove from client_sockets vector
    std::vector<int>::iterator it = std::find(client_sockets.begin(), client_sockets.end(), clientFd);
    if (it != client_sockets.end()) {
        client_sockets.erase(it);
    }
    // Find and remove the corresponding Client object
    for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if ((*it)->getFD() == clientFd)
        {
            delete *it;  // Clean up the Client object
            clients.erase(it);
            break;
        }
    }
}

/*Check for timed-out clients and remove them*/
void    HttpServer::CheckTimeouts()
{
    std::vector<int> timedOutClients;
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i]->isTimedOut())
        {
            timedOutClients.push_back(clients[i]->getFD());
        }
    }
    // Remove timed-out clients
    for (size_t i = 0; i < timedOutClients.size(); i++)
    {
        std::cout << "[DEBUG] Client " << timedOutClients[i] << " has timed out, removing connection" << std::endl;
        RemoveClient(timedOutClients[i]);
    }
}

/*The server run here*/
void    HttpServer::StartServer()
{
    int     S_status;           //To check The status of select()
    fd_set  ReadableFds;        // The FDs that are ready for reading
    fd_set  WritableFds;        // The FDs that are ready for writing
    int     MaxFds;
    int     remaining_activity;

    while(true)
    {
        MaxFds = 0;
        FD_ZERO(&ReadableFds);
        FD_ZERO(&WritableFds);
        AddToSet(&ReadableFds,&WritableFds, &MaxFds);
        S_status = select(MaxFds + 1, &ReadableFds, &WritableFds, NULL, NULL);
        if(S_status == -1)  //select fails
        {
            SelectFails();
            continue;
        }
        remaining_activity = S_status;
        CheckListeningSocket(&ReadableFds, &remaining_activity);
        CheckReadableClients(&ReadableFds);
        CheckWriteableClients(&WritableFds);
        CheckTimeouts();
    }
}