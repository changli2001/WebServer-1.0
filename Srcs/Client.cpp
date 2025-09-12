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



/*Enhanced HTTP request reading with proper buffering*/
bool Client::readAndParseRequest()
{
    // For echo server, we'll use the echo methods instead
    return true;
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

/*Check if headers are complete in tmpBuff and extract them*/
bool Client::checkHeadersComplete()
{
    // Look for the end of headers marker: \r\n\r\n
    size_t headers_end = tmpBuff.find("\r\n\r\n");
    
    if (headers_end != std::string::npos)
    {
        // Headers are complete - extract them (including the request line)
        Headers = tmpBuff.substr(0, headers_end + 2); // Include the final \r\n but not the separator
        // Update parsing state to indicate headers are parsed
        currentParsingState = BODYPARSE;
        // Call parseheaders method to process the extracted headers
        parseheaders();
        // Remove the headers part from tmpBuff, keeping the body part
        tmpBuff = tmpBuff.substr(headers_end + 4); // Skip the \r\n\r\n
        return true; // Headers are complete and extracted
    }
    // Headers are not yet complete
    return false;
}

/*Handle echo read - read data from client and store it*/
bool Client::readClientRequest()
{
    
    char buffer[40000];
    while (true)
    {
        ssize_t bytes_read = recv(ClientFD, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0';
            tmpBuff += std::string(buffer, bytes_read);
            updateActivity();
            std::cout << YELLOW << "[DEBUG]: " << RESET << "read  :" << bytes_read << "From client ."<< std::endl;
            //printt what readed
            std::cout << BLUE << "[DEBUG]: " << RESET << "Content :" << tmpBuff << std::endl;
            //break; // Exit loop after reading available data
        }

        else if(bytes_read == -1)
        {
            // std::cout << RED << "Client Connection Fails .." << RESET << std::endl;
            return true;
        }
        else if(bytes_read == 0)
        {
            std::cout << RED << "Client Disconnect .." << RESET << std::endl;
            return false;
        }
    }
    return true;
}


// Destructor 
Client::~Client()
{
    if (ClientFD != -1) {
        close(ClientFD);
    }
}

/*Handle echo write - send echo response back to client*/
bool Client::handleEchoWrite()
{
    // Simple echo response - just send back what we received
    std::stringstream ss;
    ss << tmpBuff.length();
    std::string response = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/plain\r\n"
                          "Content-Length: " + ss.str() + "\r\n"
                          "Connection: close\r\n"
                          "\r\n" + tmpBuff;
    
    ssize_t bytes_sent = send(ClientFD, response.c_str(), response.length(), 0);
    if (bytes_sent == -1)
    {
        std::cout << RED << "Failed to send echo response" << RESET << std::endl;
        return false;
    }
    
    updateActivity();
    std::cout << GREEN << "Echo response sent: " << bytes_sent << " bytes" << RESET << std::endl;
    
    // Clear the buffer after echoing
    tmpBuff.clear();
    
    return true;
}