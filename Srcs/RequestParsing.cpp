//Were the received request will be parsed
#include "../Includes/Client.hpp"

/*The methode receive a part of the client request and check if there is a /r/n/r/n */

/*Parse the HTTP headers after they have been extracted*/
void Client::parseheaders()
{
    // TODO: Implement header parsing logic
    // This method should parse the Headers string and extract:
    // - HTTP method, path, and version from the request line
    // - Individual header fields and their values
    // - Content-Length, Content-Type, etc.
    
    // Placeholder implementation
    std::cout << "[DEBUG] parseheaders() called - headers extracted: " << Headers.length() << " bytes" << std::endl;
}