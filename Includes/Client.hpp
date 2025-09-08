#pragma once

#include <string>
#include <ctime>

#define CRLF "\r\n"
// Forward declaration to avoid circular dependency
class HttpServer;

struct SClientRequest
{
    std::string         	clientMethode;      /*What the client is requesting from our server ?? GET {Other methods coming soon}*/
    std::string         	clientSourceReq;    /*What the client wants from our server ??*/
    std::string         	httpVersion;        /*What the type of HTTP protocol the client is using?*/
    std::string         	completURL;         /*The complete final URL that should be served to the client */
    
    char                	Request[8192];      // Increased buffer size for complete requests
    char                	ReqMethode[70];
    std::string				LocalisationName;
    char                	HttpVer[15];
    
    // Enhanced request handling
    std::string             rawRequest;         // Store complete raw request
    std::string             headers;            // Store HTTP headers
    std::string             body;               // Store HTTP body
    bool                    isComplete;         // Flag to check if request is complete
    size_t                  contentLength;      // Content-Length from headers
    bool                    hasBody;            // Flag to check if request has body
};

class   Client{
    private:
        int                    ClientFD;
        struct SClientRequest  request;
        std::string           clientIP;
        time_t                lastActivity;     // Track last activity time
        static const int      TIMEOUT_SECONDS = 30;  // 30 second timeout
        // HTTP Response components
        std::string           finalResponse;
        std::string           responseStartLine;
        std::string           responseHeaders;
        std::string           responseBody;
        unsigned int          statusNumber;
        std::string           statusDescription;
        std::string           defaultHtmlPage;
        // HTTP Request parsing methods (enhanced)
        void storeMethode();                    // Parse HTTP method
        void storeRequestedPath();              // Parse requested path  
        void storeHttpVersion();                // Parse HTTP version
        void checkClientMethode();              // Validate HTTP method
        bool parseHeaders();                    // Parse HTTP headers
        bool isRequestComplete();               // Check if request is complete
        size_t getContentLengthFromHeaders();   // Extract Content-Length
        bool readRequestBody();                 // Read HTTP body if present
        // HTTP Response generation methods
        void genStartLine();                    // Generate status line
        void genHeaders(const std::string& location = "");  // Generate headers
        void genBody();                         // Generate response body
        std::string getStatusDescription(unsigned int status);
        
    public:
        Client(int fd);
        ~Client();
        // Getters
        int getFD() const;
        const std::string& getIP() const;
        time_t getLastActivity() const;
        const std::string& getMethod() const;
        const std::string& getPath() const;
        const std::string& getHttpVersion() const;
        
        // Timeout management
        void updateActivity();                  // Update last activity time
        bool isTimedOut() const;               // Check if client has timed out
        
        // HTTP Request/Response processing (enhanced from your methods)
        bool readAndParseRequest();             // Read and parse complete HTTP request
        void processRequest();                  // Process the parsed request
        void processAdvancedRequest();          // Advanced request processing with file serving
        void sendHttpDefaultPage();            // Send default HTTP page
        void sendErrorResponse(unsigned int errorCode);  // Send error response
        void sendResponseToClient();           // Send final response to client
        void serveRegularFile(const std::string& filePath);  // Serve static files
        void sendRedirection301(const std::string& newUrl);  // Send 301 redirect
        
        // Request handling (legacy)
        void setRequest(const SClientRequest& req);
        const SClientRequest& getRequest() const;
        
        // Client operations
        void sendResponse(const std::string& response);
        bool readRequest();
};

