#pragma once

#include <string>
#include <ctime>
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
#include "colors.hpp"
#include <map>
#include <vector>
#define CRLF "\r\n"
#define BUFFERSIZE 1024
// Forward declaration to avoid circular dependency
class HttpServer;

struct SClientRequest
{
    std::string         	clientMethode;      /*What the client is requesting from our server ?? GET {Other methods coming soon}*/
    std::string         	clientSourceReq;    /*What the client wants from our server ??*/
    std::string         	httpVersion;        /*What the type of HTTP protocol the client is using?*/
    std::string         	completURL;         /*The complete final URL that should be served to the client */
    std::string				LocalisationName;
    
    // Enhanced request handling
    std::string             rawRequest;         // Store complete raw request
    std::string             headers;            // Store HTTP headers (raw form)
    std::string             body;               // Store HTTP body
    bool                    isComplete;         // Flag to check if request is complete
    size_t                  contentLength;      // Content-Length from headers
    bool                    hasBody;            // Flag to check if request has body

    // Extra parsed info
    std::map<std::string, std::string> parsedHeaders; // parsed headers for easier access
};


enum ParsingState {
    HEADERSPARS,      // Client is ready to read incoming data
    BODYPARSE,     // Client is processing the request
    BADREQUEST,
    VALIDREQUEST
};

// Client state enumeration for I/O management
enum ClientState {
    READSTATE,      // Client is ready to read incoming data
    PROCESSING,     // Client is processing the request
    WRITESTATE,     // Client has data ready to write
    CLOSE           // Client should be closed/disconnected
};

class   Client{
    private:
        int                    ClientFD;
        struct SClientRequest  request;
        std::string           clientIP;
        std::string           tmpBuff;          /*A temporaire buffer to use*/
        time_t                lastActivity;     // Track last activity time
        static const int      TIMEOUT_SECONDS = 30;  // 30 second timeout
        ClientState           currentState;     // Current client state for I/O operations
        
        // HTTP Response components
        std::string           finalResponse;
        std::string           responseStartLine;
        std::string           responseHeaders;
        std::string           responseBody;
        size_t                bytesSent;        // Track how many bytes have been sent

        //PARSING REQUEST
        bool                  REQUESTSTATE; // if the request is valid or not 
        ParsingState          currentParsingState; // ParsingHeader or Body !

        unsigned int          statusNumber;
        std::string           statusDescription;
        std::string           defaultHtmlPage;
        std::string           Headers;          /*At this variable we will accum the headers till /r/n/r/n*/
        // HTTP Request parsing methods (enhanced)
        void checkClientMethode();              // Validate HTTP method
        bool isRequestComplete();               // Check if request is complete
        size_t getContentLengthFromHeaders();   // Extract Content-Length
        bool readRequestBody();                 // Read HTTP body if present
        void parseheaders();                    // Parse extracted HTTP headers
        // HTTP Response generation methods
        void genStartLine();                    // Generate status line
        void genHeaders(const std::string& location = "");  // Generate headers
        void genBody();                         // Generate response body
        std::string getStatusDescription(unsigned int status);
        
    public:
        Client(int fd);
        ~Client();
        
        // Getters
        int                         getFD() const;
        time_t                      getLastActivity() const;
        const std::string&          getMethod() const;
        const std::string&          getPath() const;
        const std::string&          getHttpVersion() const;
        ParsingState                getParseState() const;
        
        // --- setters ----
        void    SetMethdode(std::string Methode);
        void    SetPath(std::string Path);
        void    SetHttpVersion(std::string httpVer);
        void    SetRequestPath(std::string Path);
        // Timeout management
        void updateActivity();                  // Update last activity time
        bool isTimedOut() const;               // Check if client has timed out
        //parse request
        int     parseRequest();                 /*-1 if the request is invalide , */
        std::string getBody() const;
        size_t getContentLength() const;
        
        void printParsedRequest() const;
        bool    checkHeadersComplete();         /*Check tmpBuff for complete headers and extract them*/
        // HTTP Request/Response processing (enhanced from your methods)
        bool readAndParseRequest();             // Read and parse complete HTTP request
        void processRequest();                  // Process the parsed request
        void sendHttpDefaultPage();            // Send default HTTP page
        void sendErrorResponse(unsigned int errorCode);  // Send error response
        void sendResponseToClient();           // Send final response to client
        void serveRegularFile(const std::string& filePath);  // Serve static files
        void sendRedirection301(const std::string& newUrl);  // Send 301 redirect
        void setfinalResponse(std::string   response);
        // State management for I/O operations
        ClientState getState() const;           // Get current client state
        void setState(ClientState newState);    // Set client state
        void setParseState(ParsingState newState); // Set current parsing state
        bool needsRead() const;                 // Check if client needs to read
        bool needsWrite() const;                // Check if client has data to write
        bool canRead() const;                   // Check if in reading state
        bool canWrite() const;                  // Check if in writing state
        
        // Request handling (legacy)
        void setRequest(const SClientRequest& req);
        const SClientRequest& getRequest() const;
        //Request parsing 
        // Client operations
        void sendResponse(const std::string& response);
        //Echo server methods
        bool readClientRequest();
        bool handleEchoWrite();
};

