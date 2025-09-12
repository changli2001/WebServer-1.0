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
        FD_SET(sockFd, ReadableClients);
        if (sockFd > *maxFd)
        {
            *maxFd = sockFd;
        }
    }
    //Add clients based on their state
    for (std::map<int, Client*>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
    {
        Client* clientObj = it->second;
        int client_fd = it->first;
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

// std::string    genreateBadRequest()
// {

// }

/*Check if a client is READABLE and handle the data*/
void         HttpServer::CheckReadableClients(fd_set  *MonitoredClients)
{
    // Iterate through all clients and check if they're ready to read
    for (std::map<int, Client*>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
    {
        int client_fd = it->first;
        Client* clientObj = it->second;
        
        if (FD_ISSET(client_fd, MonitoredClients) && clientObj->getState() == READSTATE)
        {
            if (!clientObj->readClientRequest())
            {
                RemoveClient(client_fd);
                continue;
            }
            //The Client parsing request is malformed;
            if(!clientObj->parseRequest() && clientObj->getParseState() == BADREQUEST) //parse completed
            {
                // Get the appropriate server config for this client's port
                ServerConfig* config = getServerConfigByClientFD(client_fd);
                std::string response = generateErrorResponse(400, config); // 400 : bad request ;
            }
            else if (!clientObj->parseRequest() && clientObj->getParseState() == VALIDREQUEST) //The request is good and we not should start generate a response :
            {
                std::string response = genreateResponse();
            }
        }
    }
}

void         HttpServer::CheckWriteableClients(fd_set  *MonitoredClients)
{
    // Iterate through all clients and check if they're ready to write
    for (std::map<int, Client*>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
    {
        int client_fd = it->first;
        Client* clientObj = it->second;
        
        if (FD_ISSET(client_fd, MonitoredClients) && clientObj->getState() == WRITESTATE)
        {
            if (!clientObj->handleEchoWrite())
            {
                std::cout << "Client " << client_fd << " echo write failed, disconnecting" << std::endl;
                RemoveClient(client_fd);
                return;  // Client was removed, exit function
            }
            else
            {
                clientObj->setState(READSTATE);
                std::cout << "Client " << client_fd << " echo sent, back to READSTATE" << std::endl;
            }
            return;  // Processed one client, exit function
        }
    }
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
    this->clients[newClient] = clientObj;  // Store the client object in map
    this->clientToListeningSocket[newClient] = ActiveFd;  // Map client FD to listening socket FD
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
    std::cout << YELLOW << "[DEBUG] Removing client " << clientFd << RESET << std::endl;
    close(clientFd);
    
    // Find and remove the Client object from map
    std::map<int, Client*>::iterator it = clients.find(clientFd);
    if (it != clients.end()) {
        delete it->second;  // Delete the Client object
        clients.erase(it);  // Remove from map
        std::cout << YELLOW << "[DEBUG] Deleted Client object for fd " << clientFd << RESET << std::endl;
    }
    
    // Remove from client-to-listening-socket mapping
    std::map<int, int>::iterator mapping_it = clientToListeningSocket.find(clientFd);
    if (mapping_it != clientToListeningSocket.end()) {
        clientToListeningSocket.erase(mapping_it);
        std::cout << YELLOW << "[DEBUG] Removed client-to-socket mapping for fd " << clientFd << RESET << std::endl;
    }
    
    std::cout << "[DEBUG] Client " << clientFd << " cleanup complete" << std::endl;
}

//eee
/*Get ServerConfig by client FD - uses the client-to-listening-socket mapping*/
ServerConfig* HttpServer::getServerConfigByClientFD(int clientFd)
{
    // Find which listening socket this client connected to
    std::map<int, int>::iterator it = clientToListeningSocket.find(clientFd);
    if (it != clientToListeningSocket.end())
    {
        int listeningSocketFd = it->second;
        int port = this->SocketsPort[listeningSocketFd];
        return getServerConfigByPort(port);
    }
    return NULL;
}

/*Get ServerConfig by port number*/
ServerConfig* HttpServer::getServerConfigByPort(int port)
{
    // Iterate through all server configurations to find matching port
    for (std::vector<ServerConfig>::iterator it = Servers.begin(); it != Servers.end(); ++it)
    {
        if (it->Port == port)
        {
            return &(*it);
        }
    }
    return NULL;
}

/*Check for timed-out clients and remove them*/
void    HttpServer::CheckTimeouts()
{
    std::vector<int> timedOutClients;
    for (std::map<int, Client*>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
    {
        if (it->second->isTimedOut())
        {
            timedOutClients.push_back(it->first);
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
    fd_set  ReadableFds;        // The FDs that are ready for reading | 1010 | 2020 | |
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
        CheckListeningSocket(&ReadableFds, &remaining_activity); // check if a new client connected 
        CheckReadableClients(&ReadableFds); // check if one of the connected clients whant to send som data
        CheckWriteableClients(&WritableFds);
        CheckTimeouts();
    }
}
