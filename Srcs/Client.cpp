#include "../Includes/Client.hpp"


Client::Client(int fd) : ClientFD(fd)
{
    // Initialize activity time
    lastActivity = time(NULL);
    // Initialize HTTP response components
    statusNumber = 0;
    statusDescription = "";
    finalResponse = "";
    responseStartLine = "";
    responseHeaders = "";
    responseBody = "";
    bytesSent = 0;
    // Initialize default HTML page (from your ClientRequest.cpp)
    defaultHtmlPage =
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <title>Welcome to webserv!</title>\n"
        "    <style>\n"
        "        body {\n"
        "            width: 35em;\n"
        "            margin: 0 auto;\n"
        "            font-family: Tahoma, Verdana, Arial, sans-serif;\n"
        "        }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "    <h1>Welcome to webserv!</h1>\n"
        "    <p>If you see this page, the webserv web server is successfully installed and\n"
        "    working. Further configuration is required.</p>\n"
        "\n"
        "    <p>For online documentation and support please refer to\n"
        "    <a href=\"http://webserv.org/\">webserv.org</a>.<br/>\n"
        "    Commercial support is available at\n"
        "    <a href=\"http://webserv.com/\">webserv.com</a>.</p>\n"
        "\n"
        "    <p><em>Thank you for using webserv.</em></p>\n"
        "</body>\n"
        "</html>";
    // Initialize request structure
    request.clientMethode = "";
    request.clientSourceReq = "";
    request.httpVersion = "";
    request.completURL = "";
    request.LocalisationName = "";
    request.rawRequest = "";
    request.headers = "";
    request.body = "";
    request.isComplete = false;
    request.contentLength = 0;
    request.hasBody = false;
    
    // Initialize client state to READSTATE (ready to read incoming data)
    currentState = READSTATE;
    currentParsingState = HEADERSPARS;  // Initialize to header parsing state
    REQUESTSTATE = true;
}


int Client::getFD() const 
{
    return ClientFD;
}

time_t Client::getLastActivity() const 
{
    return lastActivity;
}

const std::string& Client::getMethod() const 
{
    return request.clientMethode;
}

const std::string& Client::getPath() const 
{
    return request.clientSourceReq;
}

const std::string& Client::getHttpVersion() const 
{
    return request.httpVersion;
}

void Client::setRequest(const SClientRequest& req) 
{
    request = req;
}

const SClientRequest& Client::getRequest() const 
{
    return request;
}

void Client::sendResponse(const std::string& response) {
    send(ClientFD, response.c_str(), response.length(), 0);
    updateActivity();  // Update activity when sending response
}

void Client::updateActivity()
{
    lastActivity = time(NULL);
}

bool Client::isTimedOut() const
{
    time_t currentTime = time(NULL);
    time_t idleTime = currentTime - lastActivity;
    return (idleTime > TIMEOUT_SECONDS);
}

/*Get current client state*/
ClientState Client::getState() const
{
    return currentState;
}

/*Get current parsing state*/
ParsingState Client::getParseState() const
{
    return currentParsingState;
}

/*Set client state*/
void Client::setState(ClientState newState)
{
    currentState = newState;
}

/*Set parsing state*/
void Client::setParseState(ParsingState newState)
{
    currentParsingState = newState;
}

/*Check if client needs to read data*/
bool Client::needsRead() const
{
    return (currentState == READSTATE);
}

/*Check if client has data ready to write*/
bool Client::needsWrite() const
{
    return (currentState == WRITESTATE && !finalResponse.empty());
}

/*Check if client can perform read operations*/
bool Client::canRead() const
{
    return (currentState == READSTATE);
}

/*Check if client can perform write operations*/
bool Client::canWrite() const
{
    return (currentState == WRITESTATE);
}




/*Check if HTTP request is complete (ends with \r\n\r\n)*/
bool Client::isRequestComplete()
{
    return (true);
}

int     Client::parseRequest()
{
    this->setParseState(BADREQUEST);
    return(0);
}


void Client::setfinalResponse(std::string response)
{
    this->finalResponse = response;
    this->bytesSent = 0;  // Reset counter when setting new response
}

/*Handle echo read - read data from client and store it*/
bool Client::readClientRequest()
{
    char buffer[40000];

        ssize_t bytes_read = recv(ClientFD, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0';
            tmpBuff += std::string(buffer, bytes_read);
            updateActivity();
            parseRequest();
        }
        else if(bytes_read == -1)
        {
            std::cout << RED << "Read error on client socket" << RESET << std::endl;
            return false;
        }
        else if(bytes_read == 0)
        {
            std::cout << RED << "Client Disconnect .." << RESET << std::endl;
            return false;
        }
    return true;
}



/*Handle echo write - send echo response back to client*/
bool Client::handleEchoWrite()
{
    if (finalResponse.empty()) {
        std::cout << RED << "No response to send" << RESET << std::endl;
        return false;
    }

    // Calculate remaining bytes to send
    size_t remainingBytes = finalResponse.length() - bytesSent;
    if (remainingBytes == 0) {
        std::cout << GREEN << "All response data sent successfully - closing connection" << RESET << std::endl;
        finalResponse.clear();
        bytesSent = 0;
        tmpBuff.clear();
        return false;  // Signal to close connection
    }

    // Send remaining data
    ssize_t bytes_sent = send(ClientFD, finalResponse.c_str() + bytesSent, remainingBytes, 0);
    if (bytes_sent == -1)
    {
        std::cout << RED << "Failed to send response" << RESET << std::endl;
        return false;
    }
    bytesSent += bytes_sent;
    updateActivity();
    std::cout << GREEN << "Sent " << bytes_sent << " bytes. Total: " << bytesSent << "/" << finalResponse.length() << RESET << std::endl;
    // Check if all data has been sent
    if (bytesSent >= finalResponse.length()) {
        std::cout << GREEN << "Complete response sent successfully - closing connection" << RESET << std::endl;
        finalResponse.clear();
        bytesSent = 0;
        tmpBuff.clear();
        return false;
    }
    std::cout << YELLOW << "Partial send - " << (finalResponse.length() - bytesSent) << " bytes remaining" << RESET << std::endl;
    return true;
}

// Destructor 
Client::~Client()
{
    if (ClientFD != -1) {
        close(ClientFD);
    }
}