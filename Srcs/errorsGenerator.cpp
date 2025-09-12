#include "../Includes/HttpServer.hpp"
#include <unistd.h>  // For access() function
#include <fstream>   // For file operations
#include <sstream>   // For stringstream
#define CRLF "\r\n"

//Get the error description depending on the error number;
std::string HttpServer::getStatusDes(unsigned int err)
{	
	switch(err)
	{
        case (404): return "Not Found";
        case (403): return "Forbidden";
        case (500): return "Internal Server Error";
        case (405): return "Method Not Allowed";
        case (505): return "HTTP Version Not Supported";
        default:  return "Error";
	}
}

/*This methode check an error page path Have read Permessions to use it */
bool HttpServer::isPageReadable() const
{
	// Check if we have a current server configuration
	if (currentServerConfig == NULL)
		return false;
		
	// Get the error pages map from current server config
	std::map<int, std::string> _error_pages = currentServerConfig->error_pages;
	std::map<int, std::string>::iterator it = _error_pages.find(this->status_nmbr);
	
	// Check if error page path exists for this status code
	if (it == _error_pages.end())
		return false;
	
	// Check if the file has read permissions
	if (access((it->second).c_str(), R_OK) != 0)
    	return false;
    	
	return true;
}

/*This methode check if an error page Path is provided*/
bool HttpServer::defaultErrPageProvided(const short Error) const
{
	// Get the error pages map from current server config
	std::map<int, std::string> _error_pages = currentServerConfig->error_pages;
	std::map<int, std::string>::iterator it = _error_pages.find(Error);

	if(it != _error_pages.end()) // found a default error page
	{
		return (true);
	}
	return (false);
}


/*This Methoe Generate The first Line */
void HttpServer::genStartLine()
{
	std::string		startLine;
	std::string		httpVersion 		= "HTTP/1.1";
	std::stringstream ss;
	ss << this->status_nmbr;
	std::string		errorCode 			= ss.str();
	std::string		ErrorDescription  	= this->statusDescription;
	
	startLine = httpVersion + " " + errorCode + " " + ErrorDescription + CRLF;
	this->_responseStartLine = startLine;
}

//HEADERS 
void HttpServer::genHeaders()
{
	std::string		content_type   = "Content-Type: text/html";
	std::string		server_name    = "Server: WebSerbise";
	std::string		connectionType = "Connection: close";

	this->_responseHeaders = content_type + CRLF + server_name + CRLF + connectionType + CRLF;
}


void HttpServer::genBody()
{
	//open the error file page.
	std::string		responseBody;
	
	//check if there is a default error page for that error
	if (this->defaultErrPageProvided(this->status_nmbr) == true)
	{
		/*can be read*/
		if(isPageReadable() == true)
		{
			// Read the custom error page file
			std::map<int, std::string> _error_pages = currentServerConfig->error_pages;
			std::map<int, std::string>::iterator it = _error_pages.find(this->status_nmbr);
			std::string input_file_err = it->second;
			
			std::ifstream inputFile(input_file_err.c_str());
			if (inputFile.is_open())
			{
				std::string line;
				while (std::getline(inputFile, line))
				{
					responseBody += line + "\n";
				}
				inputFile.close(); // Close the file
				this->_responseBody = responseBody;
			}
			else
			{
				// File couldn't be opened, fall back to default
				this->_responseBody = returnStatusPageHTML(this->status_nmbr);
			}
		}
		else if (!isPageReadable())
		{
			this->status_nmbr = 500;
			this->statusDescription = getStatusDes(this->status_nmbr);
			genStartLine();
			if(!isPageReadable())
			{
				this->_responseBody =  returnStatusPageHTML(this->status_nmbr);
				return ;
			}
			genBody();
			return ;
		}
	}
	else
	{
		//return a default error page depending on the erro number;
		this->_responseBody =  returnStatusPageHTML(this->status_nmbr);
	}
}

/*Returning a defined Html code , depending on the error number !*/
std::string HttpServer::returnStatusPageHTML(unsigned short error) const
{
	switch (error)
	{
	case (403):
		return "<html><head><title>403 Forbidden</title></head><body><center><h1>403 Forbidden</h1></center><hr>"
		"<center>WebSerbise/x.x.x</center>"
	"</body></html>";
	case (405):
		return "<html><head><title>405 Methode Not Allowed</title></head><body><center><h1>405 Methode Not Allowed</h1></center><hr>"
		"<center>WebSerbise/x.x.x</center></body></html>";
	case(505):
		return "<html><head><title>505 HTTP Version Not Supported</title></head><body><center><h1>505 HTTP Version Not Supported</h1></center><hr>"
		"<center>WebSerbise/x.x.x</center></body></html>";
	case(404):
		return "<html><head><title>404 Not Found</title></head><body><center><h1>404 Not Found</h1></center><hr>"
		"<center>WebSerbise/x.x.x</center></body></html>";
	case(500):
		return "<html><head><title>500 Internal Server Error</title></head><body><center><h1>500 Internal Server Error</h1></center><hr>"
		"<center>WebSerbise/x.x.x</center></body></html>";
	default:
		return "<html><body><h1>404 Not Found</h1></body></html>";
	}
}

/*Set current server config for error generation*/
void HttpServer::setCurrentServerConfig(ServerConfig* config)
{
    currentServerConfig = config;
}

/*This Methode is responsible of creating a response for a client */
std::string HttpServer::generateErrorResponse(int ErrorCode) 
{
	// Set the error code and description
	this->status_nmbr = ErrorCode;
	this->statusDescription = getStatusDes(ErrorCode);
	
	// Generate all parts of the response
	genStartLine();
	genHeaders();
	genBody();
	
	// Add Content-Length header
	std::stringstream lengthStream;
	lengthStream << _responseBody.size();
 	std::string lenght = "Content-Length: " + lengthStream.str() + CRLF;
    this->_responseHeaders += lenght;
    // Assemble the complete response
    this->finalResponse = this->_responseStartLine + this->_responseHeaders + CRLF + this->_responseBody + CRLF;
    return this->finalResponse;
}

/*Generate error response with specific server config*/
std::string HttpServer::generateErrorResponse(int ErrorCode, ServerConfig* serverConfig)
{
    // Set the server config for this error response
    setCurrentServerConfig(serverConfig);
    
    // Generate the response
    return generateErrorResponse(ErrorCode);
}

/*Generate regular HTTP response*/
std::string HttpServer::genreateResponse()
{
    // TODO: Implement proper HTTP response generation
    // For now, return a simple response
    std::string response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 44\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body><h1>Hello World!</h1></body></html>";
    
    return response;
}

