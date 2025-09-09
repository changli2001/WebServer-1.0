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

/*Set client state*/
void Client::setState(ClientState newState)
{
    currentState = newState;
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

/*Handle echo read - read data from client and store it*/
bool Client::handleEchoRead()
{
    char buffer[1024];
    ssize_t bytes_read = recv(ClientFD, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        tmpBuff += std::string(buffer, bytes_read);
        updateActivity();
        std::cout << "Echo read: received " << bytes_read << " bytes from client " << ClientFD << std::endl;
        return true;
    }
    std::cout << "Echo read failed for client " << ClientFD << std::endl;
    return false;
}

/*Handle echo write - send stored data back to client*/
bool Client::handleEchoWrite()
{
    if (tmpBuff.empty()) {
        std::cout << "No data to echo for client " << ClientFD << std::endl;
        return true;  // Nothing to send is not an error
    }
    
    ssize_t bytes_sent = send(ClientFD, tmpBuff.c_str(), tmpBuff.size(), 0);
    
    if (bytes_sent > 0) {
        updateActivity();
        std::cout << "Echo write: sent " << bytes_sent << " bytes to client " << ClientFD << std::endl;
        tmpBuff.clear();  // Clear buffer after successful send
        return true;
    }
    std::cout << "Echo write failed for client " << ClientFD << std::endl;
    return false;
}

// Destructor 
Client::~Client()
{
    if (ClientFD != -1) {
        close(ClientFD);
    }
}