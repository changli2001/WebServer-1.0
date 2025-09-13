#include "../Includes/Client.hpp"

// Extract body from tmpBuff after headers
// std::string Client::getBody() const
// {
//     if (request.hasBody)


//     return request.body;
// }

// // Get Content-Length from parsed headers
// size_t Client::getContentLength() const
// {
//     return request.contentLength;
// }

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




/*Check if HTTP request is complete (ends with \r\n\r\n)*/
bool Client::isRequestComplete()
{
    return (true);
}

int Client::parseRequest()
{
    if (!request.isComplete)
    {
        this->setParseState(BADREQUEST);
        return -1;
    }

    // Validate method
    if (request.clientMethode != "GET" &&
        request.clientMethode != "POST" &&
        request.clientMethode != "DELETE")
    {
        this->setParseState(BADREQUEST);
        return -1;
    }

    // Validate HTTP version
    if (request.httpVersion != "HTTP/1.1")
    {
        this->setParseState(BADREQUEST);
        return -1;
    }

    this->setParseState(VALIDREQUEST);
    return 0;
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

/*Enhanced HTTP request reading with proper buffering*/
bool Client::readAndParseRequest()
{
    size_t headers_end = tmpBuff.find("\r\n\r\n");
    if (headers_end == std::string::npos)
        return false;

    std::string headerBlock = tmpBuff.substr(0, headers_end);
    tmpBuff = tmpBuff.substr(headers_end + 4); // remove headers

    std::istringstream stream(headerBlock);
    std::string line;

    // Parse request line
    if (!std::getline(stream, line))
    {
        this->setParseState(BADREQUEST);
        return true;
    }

    if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1, 1);

    std::istringstream requestLine(line);
    std::string method, url, version;
    if (!(requestLine >> method >> url >> version))
    {
        this->setParseState(BADREQUEST);
        return true;
    }

    request.clientMethode   = method;
    request.clientSourceReq = url;
    request.httpVersion     = version;

    // Parse headers
    request.headers.clear();
    while (std::getline(stream, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1, 1);
        request.headers += line + "\n";
    }

    request.hasBody = false;
    request.contentLength = 0;
    request.isComplete = true;

    // If POST, check Content-Length and Content-Type, and parse body
    if (request.clientMethode == "POST") {
        std::string contentType, host, userAgent, accept, connection;
        std::istringstream headerStream(request.headers);
        while (std::getline(headerStream, line)) {
            if (!line.empty() && line[line.size() - 1] == '\n')
                line.erase(line.size() - 1, 1);
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                while (!value.empty() && (value[0] == ' ' || value[0] == '\t')) value.erase(0, 1);
                if (key == "Content-Length") {
                    request.contentLength = std::strtoul(value.c_str(), NULL, 10);
                    if (request.contentLength > 0)
                        request.hasBody = true;
                }
                if (key == "Content-Type") {
                    contentType = value;
                }
                if (key == "Host") {
                    host = value;
                }
                if (key == "User-Agent") {
                    userAgent = value;
                }
                if (key == "Accept") {
                    accept = value;
                }
                if (key == "Connection") {
                    connection = value;
                }
                if (key == "Referer") {
                    // Store referer header
                    // Example: referer = value;
                }
                if (key == "Accept-Encoding") {
                    // Store accept-encoding header
                    // Example: acceptEncoding = value;
                }
                if (key == "Accept-Language") {
                    // Store accept-language header
                    // Example: acceptLanguage = value;
                }
                if (key == "Cache-Control") {
                    // Store cache-control header
                    // Example: cacheControl = value;
                }
                if (key == "Pragma") {
                    // Store pragma header
                    // Example: pragma = value;
                }
                if (key == "Upgrade-Insecure-Requests") {
                    // Store upgrade-insecure-requests header
                    // Example: upgradeInsecureRequests = value;
                }
            }
        }

        // Parse body if Content-Length > 0
        if (request.hasBody && request.contentLength > 0) {
            request.body = tmpBuff.substr(0, request.contentLength);
        }
        // Example: request.host = host; request.userAgent = userAgent; etc. (add to struct if needed)
    }

    parseRequest();
    return true;
}


void Client::setfinalResponse(std::string   response)
{
    this->finalResponse = response;
}

/*Handle echo read - read data from client and store it*/
bool Client::readClientRequest()
{
    char buffer[40000];
    ssize_t bytes_read = recv(ClientFD, buffer, sizeof(buffer)-1, 0);

    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        tmpBuff += std::string(buffer, bytes_read);
        updateActivity();

        if (!readAndParseRequest())
            return true; // wait for more data

        parseRequest();
    }
    else if (bytes_read == -1)
    {
        std::cout << RED << "Read error on client socket" << RESET << std::endl;
        return false;
    }
    else if (bytes_read == 0)
    {
        std::cout << RED << "Client disconnected" << RESET << std::endl;
        return false;
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
    std::string response = this->finalResponse;
    std::stringstream ss;
    ss << response.length();
    
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
