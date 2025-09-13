//here were we handlle The GET commande
#include "../Includes/HttpServer.hpp"
#include "../Includes/Client.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <sstream>
/// ---- MIME -----
/*This function is responsible of creatin a Default HTTP response 
    and send it to the Client */
std::string        HttpServer::ReturnHttpDefaultPage()
{
    std::string         body;
    this->status_nmbr = 200;

    genStartLine();
    genHeaders("");
    this->_responseBody =             "<!DOCTYPE html>\n"
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
           "    <p><em>Thank you for using webserv.</em></p>\n"
           "</body>\n"
           "</html>";
    std::ostringstream oss;
    oss << _responseBody.size();
    std::string     length = "Content-Length: " + oss.str() + CRLF;
    this->_responseHeaders += length;
    this->finalResponse = this->_responseStartLine + this->_responseHeaders + CRLF + this->_responseBody + CRLF;
    return (this->finalResponse);
}

/*when called It return a list Of MIME types */
std::map<std::string, std::string> createMimeTypeMap() 
{
    std::map<std::string, std::string> mimeTypes;
    
    // Text files
    mimeTypes["html"] = "text/html";
    mimeTypes["htm"] = "text/html";
    mimeTypes["css"] = "text/css";
    mimeTypes["js"] = "application/javascript";
    mimeTypes["json"] = "application/json";
    mimeTypes["txt"] = "text/plain";
    mimeTypes["csv"] = "text/csv";
    mimeTypes["jpg"] = "image/jpeg";
    mimeTypes["jpeg"] = "image/jpeg";
    mimeTypes["png"] = "image/png";
    mimeTypes["gif"] = "image/gif";
    mimeTypes["ico"] = "image/x-icon";
    mimeTypes["m4a"] = "audio/mp4";
    mimeTypes["mp4"] = "video/mp4";    
    mimeTypes["pdf"] = "application/pdf";    
    mimeTypes["php"] = "application/x-httpd-php";
    mimeTypes["py"] = "text/x-python";
    return mimeTypes;
}

// Helper function to get MIME type
std::string getMimeType(const std::string& filePath)
{
    static std::map<std::string, std::string> mimeTypes = createMimeTypeMap();

    // Find the last dot in the filename
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "application/octet-stream"; // Default for unknown types
    }
    
    // Extract extension and convert to lowercase
    std::string extension = filePath.substr(dotPos + 1);
    for (size_t i = 0; i < extension.length(); ++i) {
        extension[i] = std::tolower(extension[i]);
    }
    
    // Look up MIME type
    std::map<std::string, std::string>::const_iterator it = mimeTypes.find(extension);
    if (it != mimeTypes.end()) {
        std::cout << "Type is : " << it->second << std::endl;
        return it->second;
    }
    return "application/octet-stream";
}

// /*This Methode Redirect The requested Path To another Path
//     sending as a response */
// void    ClientRequest::redirection301(const std::string&    URl)
// {
//     std::string     newURL;

//     this->connectionStatus = -1;
    
//     newURL = URl + '/'; //--> /error/

//     std::cout << "New Redirect URL => " << newURL << std::endl;
    
//     this->status_nmbr = 301;
//     genStartLine();
//     genHeaders(newURL);
//     this->_responseBody = "<html>"
//         "<head>"
//          "<title>301 Moved Permanently</title>"
//         "</head>"
//         "<body>"
//          "<center>"
//                 "<h1>301 Moved Permanently</h1>"
//             "</center>"
//             "<hr>"
//             "<center>WebSerbise/x.x.x</center>"
//         "</body>"
//         "</html>;";
    
//     std::string     lenght = "Content-Length: " + std::to_string(_responseBody.size()) + CRLF;
//     this->_responseHeaders += lenght;
//     this->_responseHeaders += "Location: " + newURL + CRLF;
//     this->finalResponse = this->_responseStartLine + this->_responseHeaders + CRLF+ this->_responseBody + CRLF;
    
//     std::cout << "Final Response : " << finalResponse << RESET << std::endl;
//     sendResponseToClient();
//     //sending The response to client and closing connection;
// }













// /*This Methode Responsible of chosing The Right Response When The requested
//     URL is a regular File*/
// void    ClientRequest::HandlRegFiles(const   std::string&    filePath)
// {
//     this->connectionStatus = -1;
//     if (access(filePath.c_str(), R_OK) != 0)
//         HandlErr    noPermession(this, 403);
//     else
//         servRegFile(filePath);
// }



/*This Methode list The content of a Directory*/
std::string    HttpServer::generateAutoindexPage(const  std::string&    filePath)
{
    this->status_nmbr = 200;
    std::ostringstream html;
    struct dirent* entry;

    genStartLine();
    genHeaders("");
    
    DIR* dir = opendir(filePath.c_str());
    if (!dir)
    {
        return generateErrorResponse(500, this->currentServerConfig);
    }

    html << "<!DOCTYPE html>\n"
         << "<html>\n"
         << "<head>\n"
         << "  <meta charset=\"UTF-8\">\n"
         << "  <title>Index of " << filePath << "</title>\n"
         << "  <style>body { font-family: Arial; }</style>\n"
         << "</head>\n"
         << "<body>\n"
         << "  <h1>Directory Listing</h1>\n"
         << "  <ul>\n";

    entry = readdir(dir);
    while (entry != NULL)
    {
        const char* name = entry->d_name;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
        {
            entry = readdir(dir);
            continue;
        }
        std::string displayName = name;
        if (entry->d_type == DT_DIR)
        {
            displayName += "/";
        }
        html << "    <li><a href=\"" << name << "\">" << displayName << "</a></li>\n";
        entry = readdir(dir);
    }
    html << "  </ul>\n"
         << "</body>\n"
         << "</html>\n";
    closedir(dir);
    this->_responseBody = html.str() + CRLF;
    std::ostringstream lengthStream;
    lengthStream << _responseBody.size();
    std::string     length = "Content-Length: " + lengthStream.str() + CRLF;
    this->_responseHeaders += length;
    this->finalResponse += this->_responseStartLine + this->_responseHeaders + CRLF + this->_responseBody + CRLF;
    return (this->finalResponse);
}



// /*This Methode return The valid Root Path To work With*/
// std::string     ClientRequest::getRoorPath(const	LocationConfig* 	ClientLocation) const
// {
    //     std::string rootPath;
    //     //check which root to use 
    //     if(ClientLocation && ClientLocation->isRoot == true)
    //         rootPath = ClientLocation->root;
    //     else
    //         rootPath = this->ServerConf.rootPath;
    //     return (rootPath);
    // }
    
    // /*This Function called To Handlle The AutoIndexing Serving To list Directories
    //     Content*/

std::string    HttpServer::autoIndexingServing(bool     autoIndexStatus, const  std::string&    filePath)
{
    if (autoIndexStatus == true)
        return generateAutoindexPage(filePath);
    else
        return generateErrorResponse(403, this->currentServerConfig);
}


std::string HttpServer::handleIndexFiles(const std::vector<std::string>& indexFiles, const  std::string&    filePath)
{
    struct stat fileStat;
    std::string path = filePath;
    std::string URL;

    for (std::vector<std::string>::const_iterator it = indexFiles.begin(); it != indexFiles.end(); ++it)
    {
        if (!path.empty() && path[path.length() - 1] != '/')
            path += "/";

        URL = path + *it;
        if (stat(URL.c_str(), &fileStat) == 0)
        {
            if (S_ISREG(fileStat.st_mode))
            {
                if (access(URL.c_str(), R_OK) == 0)
                {
                    return servRegFile(URL);
                }
                else
                {
                    return generateErrorResponse(403, this->currentServerConfig);
                }
            }
        }
        if(this->currentServerConfig->autoindex == false)
        {
            return generateErrorResponse(404, this->currentServerConfig);
        }
        else
        {
            return autoIndexingServing(this->currentServerConfig->autoindex, filePath);
        }
    }
    return generateErrorResponse(404, this->currentServerConfig);
}


std::string    HttpServer::HandleDirectories(const  std::string&    filePath, ServerConfig* currentServerConfig,
                                                const LocationBlockConfig* locationBlock)
{
    if (currentServerConfig->locationMatchIndex == -1)
    {
        if(currentServerConfig->IsIndexed == true)
        {
            return handleIndexFiles(currentServerConfig->Indexes, filePath);
        }
        else if (currentServerConfig->IsAutoIndex == true)
        {
            return autoIndexingServing(currentServerConfig->IsAutoIndex, filePath);
        }
        else
        {
            return HandleRegularFiles(filePath + "index.html");
        }
    }
    //Location case block
    else
    {
        if (locationBlock && locationBlock->IsBIndexed == true)
            return handleIndexFiles(locationBlock->Indexes, filePath);        /*Passing The indxing List of The Locatio Block*/
        else if (locationBlock->IsBIndexed == false
                    && currentServerConfig->IsIndexed == true)
        {
            return handleIndexFiles(currentServerConfig->Indexes, filePath);  /*Passing The indxing List of The root Path*/
        }
        else if (locationBlock && locationBlock->IsBIndexed == false
                    && currentServerConfig->IsIndexed == false)
        {
			return autoIndexingServing(currentServerConfig->IsAutoIndex, filePath);
        }
    }
    return generateErrorResponse(500, this->currentServerConfig); // Internal Server Error
}

/*Generate HTTP headers*/
void    HttpServer::genHeaders(const    std::string path)
{
    std::string content_type = "Content-Type: ";
    
    if (!path.empty())
    {
        content_type += getMimeType(path);
    }
    else
    {
        content_type += "text/html";
    }
	content_type   = "Content-Type: text/html";
	std::string		server_name    = "Server: WebSerbise";
	std::string		connectionType = "Connection: close";
    
    this->_responseHeaders = content_type + CRLF;
    this->_responseHeaders += server_name + CRLF;
    this->_responseHeaders += connectionType + CRLF;
    return ;
}

/*when The request is For a regular existig file*/
std::string    HttpServer::servRegFile(const    std::string&    filePath)
{
    this->status_nmbr = 200;
    genStartLine();
    genHeaders(filePath);

    std::ifstream inputFile(filePath.c_str(), std::ios::binary);
    if (inputFile.is_open())
    {
        // C++98 compatible way to read binary file
        inputFile.seekg(0, std::ios::end);
        std::streamsize size = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);
        
        std::string content;
        content.resize(size);
        inputFile.read(&content[0], size);
        inputFile.close();
        
        this->_responseBody = content;
    }
    else
    {
        return generateErrorResponse(500, this->currentServerConfig);
    }
    
    std::ostringstream lengthStream;
    lengthStream << _responseBody.size();
    std::string length = "Content-Length: " + lengthStream.str() + CRLF;
    this->_responseHeaders += length;
    this->finalResponse = this->_responseStartLine + this->_responseHeaders + CRLF + this->_responseBody + CRLF;
    
    return this->finalResponse;
}

/*This Methode Responsible of chosing The Right Response When The requested
    URL is a regular File*/
std::string    HttpServer::HandleRegularFiles(const   std::string&    filePath)
{
    if (access(filePath.c_str(), R_OK) != 0)
        return generateErrorResponse(405, this->currentServerConfig);
    else
        return (servRegFile(filePath));
}


// /*This Methode Return The state of a PATH
int    HttpServer::getPathStatus(const  std::string&    pathname)
{
    struct stat fileStat;

    if (stat(pathname.c_str(), &fileStat) == 0)
    {
        if(S_ISREG(fileStat.st_mode))
        {
            return (ISREG);
        }
        else if (S_ISDIR(fileStat.st_mode))
        {
            return (ISDIRE);
        }
        else
        {
            return (NOTVALID);
        }
    }
    return (NOTFOUND);
}

// /*This method checks if the requested HTTP method is allowed for a specific location*/
bool                    HttpServer::isMethodAllowedForLocation(const LocationBlockConfig* locationBlock, std::string requestedMethod) const
{
    if (!locationBlock || !locationBlock->IsMethodsSet){
        return true;
    }    
    for (std::vector<std::string>::const_iterator it = locationBlock->allowedMethods.begin(); 
         it != locationBlock->allowedMethods.end(); ++it)
    {
        if (*it == requestedMethod)
        {
            return true;
        }
    }
    return false;
}

// /*Handle the case when a location matches with the requested PATH*/
std::string HttpServer::MachingLocationHandler(ServerConfig* currentServerConfig, Client* ClientObj)
{
    std::string             toServeUrl;
    int                     file_type;
    std::string             rootPath;
    std::string             RequestedPath = ClientObj->getPath();
    LocationBlockConfig*    locationBlock;


    locationBlock = &currentServerConfig->locations[currentServerConfig->locationMatchIndex];
    if (!isMethodAllowedForLocation(locationBlock, ClientObj->getMethod()))
    {
        return generateErrorResponse(405, this->currentServerConfig); // Method Not Allowed
    }
    if (locationBlock && locationBlock->IsBlockRoot)
    {
        rootPath = locationBlock->BlockRootPath;
    }
    else if (this->currentServerConfig->IsRootSet)
    {
        rootPath = this->currentServerConfig->RootPath;
    }
    else
    {
        return generateErrorResponse(500, this->currentServerConfig); // Internal Server Error
    }
    toServeUrl = rootPath + RequestedPath;
    file_type = getPathStatus(toServeUrl);
    if (file_type == NOTFOUND){  
        return generateErrorResponse(404, this->currentServerConfig);
    }
    else if (file_type == NOTVALID){
        return generateErrorResponse(403, this->currentServerConfig);
    }
    else if (file_type == ISREG){
        return HandleRegularFiles(toServeUrl);
    }
    else{
        return HandleDirectories(toServeUrl, currentServerConfig, locationBlock);
    }
}

std::string HttpServer::GetMethodeExec(Client *ClientObj)
{
    // Get the requested path (not method!)
    std::string RequestedPath = ClientObj->getPath();
    std::string location_path;
    int         bestMatchPos = -1;
    int         pos = 0;
    int         longMatchLength = -1;
    
    for (std::vector<LocationBlockConfig>::const_iterator it = this->currentServerConfig->locations.begin(); 
         it != this->currentServerConfig->locations.end(); ++it)
    {
        location_path = it->LocationName;
        if (location_path == RequestedPath)
        {   
            bestMatchPos = pos;
            break;
        }
        if (RequestedPath.rfind(location_path, 0) == 0) // RequestedPath starts with location_path
        {
            if (static_cast<int>(location_path.length()) > longMatchLength)
            {
                longMatchLength = location_path.length();
                bestMatchPos = pos;
            }
        }
        pos++;
    }
    if (bestMatchPos == -1)
    {
        std::cout << GREEN << "[DEBUG] " << RESET << "No matching location block found for path: " 
                  << RequestedPath << RESET << std::endl;
        // No matching location found - use server root
        // NoMachingLocationHandler(NULL);
    }
        std::cout << GREEN << "[DEBUG] " << RESET << "Best matching location index: " 
                  << bestMatchPos << " for path: " << RequestedPath << RESET << std::endl;
        this->currentServerConfig->locationMatchIndex = bestMatchPos;
        this->currentServerConfig->locationFounded = true;
        return MachingLocationHandler(this->currentServerConfig, ClientObj);
}