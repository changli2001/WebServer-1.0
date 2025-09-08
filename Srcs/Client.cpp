#include "../Includes/Client.hpp"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctime>
#include <errno.h>
#include <cstring>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>

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

bool Client::readRequest()
{
    // Implementation for reading HTTP request
    char buffer[1024];
    ssize_t bytes_read = recv(ClientFD, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        request.rawRequest += std::string(buffer, bytes_read);
        updateActivity();
        return true;
    }
    else if (bytes_read == 0)
    {
        return false;
    }
    else {
        // Error occurred or would block
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available right now, but connection is still valid
            return true;
        } else {
            // Real error occurred
            std::cout << "Error reading from client " << ClientFD << ": " << strerror(errno) << std::endl;
            return false;
        }
    }
}

// ===============================================
// HTTP REQUEST PARSING METHODS (from your ClientRequest.cpp)
// ===============================================

/*Enhanced HTTP request reading with proper buffering*/
bool Client::readAndParseRequest()
{
    const size_t BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    
    ssize_t bytes_read = recv(ClientFD, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        updateActivity();
        request.rawRequest += std::string(buffer, bytes_read);
        
        std::cout << "Received " << bytes_read << " bytes. Total: " << request.rawRequest.size() << " bytes" << std::endl;
        // Check if request is complete
        if (isRequestComplete())
        {
            std::cout << "Complete request received:" << std::endl;
            std::cout << request.rawRequest << std::endl;
            
            // Parse the complete request
            if (parseHeaders()) {
                storeMethode();
                storeRequestedPath();
                storeHttpVersion();
                
                // If request has body, try to read it
                if (request.hasBody && request.body.size() < request.contentLength) {
                    return readRequestBody();
                }
                
                // Request is complete, change to PROCESSING state
                currentState = PROCESSING;
                return true;
            }
        }
        // Request incomplete, need more data
        return true;
    }
    else if (bytes_read == 0) {
        std::cout << "Client " << ClientFD << " disconnected gracefully" << std::endl;
        return false;
    }
    else {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return true; // No data available, but connection is still valid
        } else {
            std::cout << "Error reading from client " << ClientFD << ": " << strerror(errno) << std::endl;
            return false;
        }
    }
}

/*Check if HTTP request is complete (ends with \r\n\r\n)*/
bool Client::isRequestComplete()
{
    // HTTP headers end with \r\n\r\n
    size_t headerEnd = request.rawRequest.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
    {
        return false; // Headers not complete yet
    }
    
    // Extract headers if not already done
    if (request.headers.empty())
    {
        request.headers = request.rawRequest.substr(0, headerEnd + 2); // Include \r\n
    }
    
    // Check if request has body
    size_t contentLength = getContentLengthFromHeaders();
    if (contentLength > 0)
    {
        request.hasBody = true;
        request.contentLength = contentLength;
        
        // Extract body so far
        size_t bodyStart = headerEnd + 4; // Skip \r\n\r\n
        if (bodyStart < request.rawRequest.size()) {
            request.body = request.rawRequest.substr(bodyStart);
        }
        // Check if we have received the complete body
        return request.body.size() >= contentLength;
    }
    
    request.isComplete = true;
    return true; // GET requests without body are complete
}

/*Parse HTTP headers and extract important information*/
bool Client::parseHeaders()
{
    if (request.headers.empty()) {
        return false;
    }
    
    std::cout << "Parsing headers:" << std::endl;
    std::cout << request.headers << std::endl;
    
    // Extract Content-Length if present
    request.contentLength = getContentLengthFromHeaders();
    if (request.contentLength > 0) {
        request.hasBody = true;
        std::cout << "Request has body with Content-Length: " << request.contentLength << std::endl;
    }
    
    return true;
}

/*Extract Content-Length from headers*/
size_t Client::getContentLengthFromHeaders()
{
    size_t contentLength = 0;
    
    // Look for Content-Length header (case insensitive)
    std::string headers_lower = request.headers;
    std::transform(headers_lower.begin(), headers_lower.end(), headers_lower.begin(), ::tolower);
    
    size_t pos = headers_lower.find("content-length:");
    if (pos != std::string::npos) {
        // Find the value after the colon
        pos += 15; // Length of "content-length:"
        while (pos < headers_lower.size() && isspace(headers_lower[pos])) {
            pos++; // Skip whitespace
        }
        
        // Extract the number
        std::string value;
        while (pos < headers_lower.size() && isdigit(headers_lower[pos])) {
            value += headers_lower[pos];
            pos++;
        }
        
        if (!value.empty()) {
            std::istringstream iss(value);
            iss >> contentLength;
        }
    }
    
    return contentLength;
}

/*Read HTTP request body if present*/
bool Client::readRequestBody()
{
    if (!request.hasBody || request.body.size() >= request.contentLength)
    {
        return true;
    }
    const size_t BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    
    ssize_t bytes_read = recv(ClientFD, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        updateActivity();
        // Append to body
        request.body += std::string(buffer, bytes_read);
        request.rawRequest += std::string(buffer, bytes_read);
        
        std::cout << "Received " << bytes_read << " bytes of body. Total body: " 
                  << request.body.size() << "/" << request.contentLength << " bytes" << std::endl;
        
        // Check if body is complete
        return request.body.size() >= request.contentLength;
    }
    else if (bytes_read == 0) {
        std::cout << "Client disconnected while reading body" << std::endl;
        return false;
    }
    else {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return true; // No data available, but connection is still valid
        } else {
            std::cout << "Error reading body from client " << ClientFD << ": " << strerror(errno) << std::endl;
            return false;
        }
    }
}

/*Parse HTTP method from request (enhanced)*/
void Client::storeMethode()
{
    size_t space_pos = request.rawRequest.find(' ');
    if (space_pos != std::string::npos) {
        request.clientMethode = request.rawRequest.substr(0, space_pos);
    }
    
    std::cout << "Parsed method: " << request.clientMethode << std::endl;
}

/*Parse requested path from request (enhanced)*/
void Client::storeRequestedPath()
{
    size_t first_space = request.rawRequest.find(' ');
    if (first_space != std::string::npos) {
        size_t second_space = request.rawRequest.find(' ', first_space + 1);
        if (second_space != std::string::npos) {
            request.clientSourceReq = request.rawRequest.substr(first_space + 1, second_space - first_space - 1);
        }
    }
    
    std::cout << "Parsed path: " << request.clientSourceReq << std::endl;
}

/*Parse HTTP version from request (enhanced)*/
void Client::storeHttpVersion()
{
    size_t first_space = request.rawRequest.find(' ');
    if (first_space != std::string::npos) {
        size_t second_space = request.rawRequest.find(' ', first_space + 1);
        if (second_space != std::string::npos) {
            size_t line_end = request.rawRequest.find('\r', second_space + 1);
            if (line_end == std::string::npos) {
                line_end = request.rawRequest.find('\n', second_space + 1);
            }
            if (line_end != std::string::npos) {
                request.httpVersion = request.rawRequest.substr(second_space + 1, line_end - second_space - 1);
            }
        }
    }
    
    std::cout << "Parsed HTTP version: " << request.httpVersion << std::endl;
}

/*Validate HTTP method (from your checkClientMethode)*/
void Client::checkClientMethode()
{
    if (request.clientMethode == "GET" || 
        request.clientMethode == "POST" || 
        request.clientMethode == "DELETE") {
        std::cout << "Valid HTTP method: " << request.clientMethode << std::endl;
    } else {
        std::cout << "Unsupported HTTP method: " << request.clientMethode << std::endl;
        sendErrorResponse(405); // Method Not Allowed
    }
}

// ===============================================
// HTTP RESPONSE GENERATION METHODS (from your ClientRequest.cpp)
// ===============================================

/*Generate HTTP status line (from your genStartLine)*/
void Client::genStartLine()
{
    std::string httpVersion = "HTTP/1.1";
    std::ostringstream oss;
    oss << statusNumber;
    std::string statusCode = oss.str();
    statusDescription = getStatusDescription(statusNumber);
    
    responseStartLine = httpVersion + " " + statusCode + " " + statusDescription + CRLF;
    std::cout << "Generated start line: " << responseStartLine;
}

/*Generate HTTP headers (from your genHeaders)*/
void Client::genHeaders(const std::string& location)
{
    responseHeaders = "Server: WebServ/1.0" + std::string(CRLF);
    responseHeaders += "Connection: close" + std::string(CRLF);
    
    if (!location.empty()) {
        responseHeaders += "Location: " + location + CRLF;
    }
}

/*Generate response body (basic implementation)*/
void Client::genBody()
{
    // This is a basic implementation - you can enhance it based on your needs
    if (statusNumber == 200) {
        responseBody = defaultHtmlPage;
    } else {
        responseBody = "<html><body><h1>Error " + getStatusDescription(statusNumber) + "</h1></body></html>";
    }
}

/*Get status description for HTTP codes (from your getStatusDes)*/
std::string Client::getStatusDescription(unsigned int status)
{
    switch(status) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        default:  return "OK";
    }
}

// ===============================================
// HTTP REQUEST/RESPONSE WORKFLOW METHODS
// ===============================================

/*Process the parsed HTTP request*/
void Client::processRequest()
{
    std::cout << "Processing request: " << request.clientMethode << " " << request.clientSourceReq << std::endl;
    
    // Validate method first
    checkClientMethode();
    
    // Process based on method and path
    if (request.clientMethode == "GET") {
        if (request.clientSourceReq == "/" || request.clientSourceReq.empty()) {
            sendHttpDefaultPage();
        } else {
            // For now, send 404 for other paths
            sendErrorResponse(404);
        }
    } else if (request.clientMethode == "POST") {
        // Handle POST requests
        sendErrorResponse(501); // Not Implemented for now
    } else {
        sendErrorResponse(405); // Method Not Allowed
    }
}

/*Send default HTTP page (from your sendHttpDefaultPage)*/
void Client::sendHttpDefaultPage()
{
    statusNumber = 200;
    genStartLine();
    genHeaders();
    responseBody = defaultHtmlPage;
    
    std::ostringstream oss;
    oss << responseBody.size();
    std::string contentLength = "Content-Length: " + oss.str() + CRLF;
    responseHeaders += contentLength;
    
    finalResponse = responseStartLine + responseHeaders + CRLF + responseBody + CRLF;
    
    // Set state to WRITESTATE instead of immediately sending
    currentState = WRITESTATE;
}

/*Send error response*/
void Client::sendErrorResponse(unsigned int errorCode)
{
    statusNumber = errorCode;
    genStartLine();
    genHeaders();
    
    responseBody = "<html><head><title>" + getStatusDescription(errorCode) + 
                   "</title></head><body><h1>" + getStatusDescription(errorCode) + 
                   "</h1></body></html>";
    
    std::ostringstream oss;
    oss << responseBody.size();
    std::string contentLength = "Content-Length: " + oss.str() + CRLF;
    responseHeaders += contentLength;
    
    finalResponse = responseStartLine + responseHeaders + CRLF + responseBody + CRLF;
    
    // Set state to WRITESTATE instead of immediately sending
    currentState = WRITESTATE;
}

/*Send final response to client (from your sendResponseToClient)*/
void Client::sendResponseToClient()
{
    std::cout << "Sending response to client " << ClientFD << std::endl;
    std::cout << "Response: " << finalResponse << std::endl;
    
    ssize_t sent = send(ClientFD, finalResponse.c_str(), finalResponse.size(), 0);
    if (sent == -1) {
        std::cout << "Error sending response: " << strerror(errno) << std::endl;
    } else {
        std::cout << "Sent " << sent << " bytes to client" << std::endl;
    }
    updateActivity();
}

// ===============================================
// ADVANCED HTTP METHODS (from your GetMethode.cpp)
// ===============================================

/*Serve a regular file (from your servRegFile)*/
void Client::serveRegularFile(const std::string& filePath)
{
    std::cout << "Serving file: " << filePath << std::endl;
    statusNumber = 200;
    genStartLine();
    genHeaders();
    
    std::ifstream inputFile(filePath.c_str(), std::ios::binary);
    if (inputFile.is_open()) {
        std::ostringstream body;
        body << inputFile.rdbuf();
        inputFile.close();
        responseBody = body.str();
        
        std::ostringstream oss;
        oss << responseBody.size();
        std::string contentLength = "Content-Length: " + oss.str() + CRLF;
        responseHeaders += contentLength;
        responseHeaders += "Content-Type: text/html" + std::string(CRLF);
        
        finalResponse = responseStartLine + responseHeaders + CRLF + responseBody + CRLF;
        sendResponseToClient();
    } else {
        std::cout << "Error: Could not open file " << filePath << std::endl;
        sendErrorResponse(500); // Internal Server Error
    }
}

/*Send 301 redirection (from your redirection301)*/
void Client::sendRedirection301(const std::string& newUrl)
{
    std::cout << "Redirecting to: " << newUrl << std::endl;
    statusNumber = 301;
    genStartLine();
    genHeaders(newUrl);
    
    responseBody = "<html>"
        "<head>"
         "<title>301 Moved Permanently</title>"
        "</head>"
        "<body>"
         "<center>"
                "<h1>301 Moved Permanently</h1>"
            "</center>"
            "<hr>"
            "<center>WebServ/1.0</center>"
        "</body>"
        "</html>";
    
    std::ostringstream oss;
    oss << responseBody.size();
    std::string contentLength = "Content-Length: " + oss.str() + CRLF;
    responseHeaders += contentLength;
    
    finalResponse = responseStartLine + responseHeaders + CRLF + responseBody + CRLF;
    sendResponseToClient();
}

/*Enhanced process request with file serving*/
void Client::processAdvancedRequest()
{
    std::cout << "Processing advanced request: " << request.clientMethode << " " << request.clientSourceReq << std::endl;
    
    // Validate method first
    checkClientMethode();
    
    if (request.clientMethode == "GET") {
        std::string requestPath = request.clientSourceReq;
        if (requestPath == "/" || requestPath.empty())
        {
            sendHttpDefaultPage();
        }
        else
        {
            // Try to serve file from website directory
            std::string filePath = "website" + requestPath;
            // Check if it's a directory (add trailing slash if needed)
            if (requestPath.back() != '/' && requestPath.find('.') == std::string::npos) {
                sendRedirection301(requestPath + "/");
                return;
            }
            
            // If it ends with /, try to serve index.html
            if (requestPath.back() == '/') {
                filePath += "index.html";
            }
            std::cout << "Trying to serve file: " << filePath << std::endl;
            // Check if file exists (basic check)
            std::ifstream testFile(filePath.c_str());
            if (testFile.good())
            {
                testFile.close();
                serveRegularFile(filePath);
            } else {
                testFile.close();
                sendErrorResponse(404); // Not Found
            }
        }
    } else if (request.clientMethode == "POST") {
        // Handle POST requests - for now return not implemented
        sendErrorResponse(501); // Not Implemented
    } else {
        sendErrorResponse(405); // Method Not Allowed
    }
}

// Destructor 
Client::~Client()
{
    if (ClientFD != -1) {
        close(ClientFD);
    }
}
