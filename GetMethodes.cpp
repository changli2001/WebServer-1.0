/*This Methode Redirect The requested Path To another Path
    sending as a response */
void    ClientRequest::redirection301(const std::string&    URl)
{
    std::string     newURL;

    this->connectionStatus = -1;
    
    newURL = URl + '/'; //--> /error/

    std::cout << "New Redirect URL => " << newURL << std::endl;
    
    this->status_nmbr = 301;
    genStartLine();
    genHeaders(newURL);
    this->_responseBody = "<html>"
        "<head>"
         "<title>301 Moved Permanently</title>"
        "</head>"
        "<body>"
         "<center>"
                "<h1>301 Moved Permanently</h1>"
            "</center>"
            "<hr>"
            "<center>WebSerbise/x.x.x</center>"
        "</body>"
        "</html>;";
    
    std::string     lenght = "Content-Length: " + std::to_string(_responseBody.size()) + CRLF;
    this->_responseHeaders += lenght;
    this->_responseHeaders += "Location: " + newURL + CRLF;
    this->finalResponse = this->_responseStartLine + this->_responseHeaders + CRLF+ this->_responseBody + CRLF;
    
    std::cout << "Final Response : " << finalResponse << RESET << std::endl;
    sendResponseToClient();
    //sending The response to client and closing connection;
}

/*This method checks if the requested HTTP method is allowed for a specific location*/
bool                    ClientRequest::isMethodAllowedForLocation(const LocationConfig* locationBlock) const
{
    if (!locationBlock || !locationBlock->isMethodsSet)
    {
        // If no location block or no methods specified, allow all methods
        return true;
    }
    
    std::string requestedMethod = this->clinentInfos.clientMethode;
    
    // Check if the requested method is in the allowed methods list
    for (std::vector<std::string>::const_iterator it = locationBlock->allowedMethods.begin(); 
         it != locationBlock->allowedMethods.end(); ++it)
    {
        if (*it == requestedMethod)
        {
            return true;
        }
    }
    
    return false; // Method not found in allowed methods
}


/*This method handles the case when a method is not allowed for the location*/
void                    ClientRequest::methodNotAllowedForLocation()
{
    std::cout << "The HTTP Method '" << this->clinentInfos.clientMethode 
              << "' is not allowed for this location" << std::endl;
    this->connectionStatus = -1;
    HandlErr    methodNotAllowed(this, 405);
    methodNotAllowed.sendResponseToClient();
    return;
}

/*when The request is For a regular existig file*/
void    ClientRequest::servRegFile(const    std::string&    filePath)
{
    std::ostringstream body;
    this->status_nmbr = 200;
    genStartLine();
    genHeaders(filePath);

    std::ifstream inputFile(filePath, std::ios::binary);
    if (inputFile.is_open())
    {
        body << inputFile.rdbuf();
        inputFile.close();
    }
    else
    {
        HandlErr    error(this, 500);
        return ;
    }
    this->_responseBody = body.str();
    std::string     length = "Content-Length: " + std::to_string(_responseBody.size()) + CRLF;
    this->_responseHeaders += length;
    this->finalResponse = this->_responseStartLine + this->_responseHeaders + CRLF + this->_responseBody + CRLF;

    sendResponseToClient();
}

/*This Methode Responsible of chosing The Right Response When The requested
    URL is a regular File*/
void    ClientRequest::HandlRegFiles(const   std::string&    filePath)
{
    this->connectionStatus = -1;
    if (access(filePath.c_str(), R_OK) != 0)
        HandlErr    noPermession(this, 403);
    else
        servRegFile(filePath);
}

/*This method checks if the requested HTTP method is allowed for a specific location*/
bool                    ClientRequest::isMethodAllowedForLocation(const LocationConfig* locationBlock) const
{
    if (!locationBlock || !locationBlock->isMethodsSet)
    {
        // If no location block or no methods specified, allow all methods
        return true;
    }
    
    std::string requestedMethod = this->clinentInfos.clientMethode;
    
    // Check if the requested method is in the allowed methods list
    for (std::vector<std::string>::const_iterator it = locationBlock->allowedMethods.begin(); 
         it != locationBlock->allowedMethods.end(); ++it)
    {
        if (*it == requestedMethod)
        {
            return true;
        }
    }
    
    return false; // Method not found in allowed methods
}

/*This method handles the case when a method is not allowed for the location*/
void                    ClientRequest::methodNotAllowedForLocation()
{
    std::cout << "The HTTP Method '" << this->clinentInfos.clientMethode 
              << "' is not allowed for this location" << std::endl;
    this->connectionStatus = -1;
    HandlErr    methodNotAllowed(this, 405);
    methodNotAllowed.sendResponseToClient();
    return;
}

/*Generate HTTP headers*/
void    ClientRequest::genHeaders(const    std::string path)
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
    
    std::string		server_name    = this->clinentInfos.serverName;
    std::string		connectionType = this->clinentInfos.connection_type;
    
    this->_responseHeaders = content_type + CRLF;
    this->_responseHeaders += server_name + CRLF;
    this->_responseHeaders += connectionType + CRLF;
    return ;
}

/*This function is responsible of creatin a Default HTTP response 
    and send it to the Client */
void        ClientRequest::sendHttpDefaultPage()
{
    std::string         body;
    this->status_nmbr = 200;

    genStartLine();
    genHeaders("");
    this->_responseBody = this->defaultHtmlPage;

    std::string     lenght = "Content-Length: " + std::to_string(_responseBody.size()) + CRLF;
    this->_responseHeaders += lenght;
    this->finalResponse = this->_responseStartLine + this->_responseHeaders + CRLF + this->_responseBody + CRLF;
    sendResponseToClient();
}



/*This Methode Redirect The requested Path To another Path
    sending as a response */
void    ClientRequest::redirection301(const std::string&    URl)
{
    std::string     newURL;

    this->connectionStatus = -1;
    
    newURL = URl + '/'; //--> /error/

    std::cout << "New Redirect URL => " << newURL << std::endl;
    
    this->status_nmbr = 301;
    genStartLine();
    genHeaders(newURL);
    this->_responseBody = "<html>"
        "<head>"
         "<title>301 Moved Permanently</title>"
        "</head>"
        "<body>"
         "<center>"
                "<h1>301 Moved Permanently</h1>"
            "</center>"
            "<hr>"
            "<center>WebSerbise/x.x.x</center>"
        "</body>"
        "</html>;";
    
    std::string     lenght = "Content-Length: " + std::to_string(_responseBody.size()) + CRLF;
    this->_responseHeaders += lenght;
    this->_responseHeaders += "Location: " + newURL + CRLF;
    this->finalResponse = this->_responseStartLine + this->_responseHeaders + CRLF+ this->_responseBody + CRLF;
    
    std::cout << "Final Response : " << finalResponse << RESET << std::endl;
    sendResponseToClient();
    //sending The response to client and closing connection;
}

/*when The request is For a regular existig file*/
void    ClientRequest::servRegFile(const    std::string&    filePath)
{
    std::ostringstream body;
    this->status_nmbr = 200;
    genStartLine();
    genHeaders(filePath);

    std::ifstream inputFile(filePath, std::ios::binary);
    if (inputFile.is_open())
    {
        body << inputFile.rdbuf();
        inputFile.close();
    }
    else
    {
        HandlErr    error(this, 500);
        return ;
    }
    this->_responseBody = body.str();
    std::string     length = "Content-Length: " + std::to_string(_responseBody.size()) + CRLF;
    this->_responseHeaders += length;
    this->finalResponse = this->_responseStartLine + this->_responseHeaders + CRLF + this->_responseBody + CRLF;

    sendResponseToClient();
}


/*This Methode Responsible of chosing The Right Response When The requested
    URL is a regular File*/
void    ClientRequest::HandlRegFiles(const   std::string&    filePath)
{
    this->connectionStatus = -1;
    if (access(filePath.c_str(), R_OK) != 0)
        HandlErr    noPermession(this, 403);
    else
        servRegFile(filePath);
}


/*when the index Directive Is ON , this methode loop throught all the available
    files and serve what can be server*/
void ClientRequest::handleIndexFiles(const std::vector<std::string>& indexFiles, const  std::string&    filePath)
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
                    servRegFile(URL);
                    return ;
                }
                else
                {
                    HandlErr   cantaccess(this, 403);
                    return ;
                }
            }
        }
    }
    if(this->ServerConf.autoindex == false)
    {
        HandlErr   cantaccess(this, 404);
    }
    else
    {
        this->connectionStatus = -1;
        autoIndexingServing(this->ServerConf.autoindex, filePath);
    }

}

/*This Methode list The content of a Directory*/
void    ClientRequest::generateAutoindexPage(const  std::string&    filePath)
{
    std::cout << "Generating listing " << std::endl;
    this->status_nmbr = 200;
    std::ostringstream html;
    struct dirent* entry;

    //Generate the first line
    genStartLine();
    genHeaders();

    std::cout << "Path To open : " <<filePath << std::endl;
    DIR* dir = opendir(filePath.c_str());
    if (!dir)
    {    
        std::cerr << "Failed to open directory: " << filePath << std::endl;
        //close The connection and ;
        this->connectionStatus = -1;
        HandlErr    internalServererr(this, 500);
        return ;
    }

    html << "<!DOCTYPE html>\n"
         << "<html>\n"
         << "<head>\n"
         << "  <meta charset=\"UTF-8\">\n"
         << "  <title>Indexs </title>\n"
         << "  <style>body { font-family: Arial; }</style>\n"
         << "</head>\n"
         << "<body>\n"
         << "  <h1>Directory Listing</h1>\n"
         << "  <ul>\n";

    entry = readdir(dir);
    while (entry    != NULL)
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
    std::string     lenght = "Content-Length: " + std::to_string(_responseBody.size()) + CRLF;
    this->_responseHeaders += lenght;
    this->finalResponse += this->_responseStartLine + this->_responseHeaders + CRLF + this->_responseBody + CRLF;
    sendResponseToClient();
}

/*This Function called To Handlle The AutoIndexing Serving To list Directories
    Content*/
void    ClientRequest::autoIndexingServing(bool     autoIndexStatus, const  std::string&    filePath)
{
    if(autoIndexStatus == true)
        generateAutoindexPage(filePath);
    else
    {
        HandlErr    noPermession(this, 403);
    }
}
/*This Methode Responsible of handlling The Response When The requested
    URL is a Directory
    The methode handlle Both cases (Matching Block and No Matching Block)
*/
void    ClientRequest::HandlDirectories(const  std::string&    filePath, const LocationConfig* locationBlock)
{
    if(this->connectionStatus == -1)
        return ;
    if(filePath[filePath.length() - 1] != '/')
    {
        redirection301(this->clinentInfos.clientSourceReq);
        return ;
    }
    // No location case block
    if (this->withLocation == false)
    {
        if(this->ServerConf.isIndexesSet == true)
        {
            this->connectionStatus = -1;
            handleIndexFiles(this->ServerConf.Indexes, filePath);
        }
        else if (this->ServerConf.autoindex == true)
        {
            this->connectionStatus = -1;
            autoIndexingServing(this->ServerConf.autoindex, filePath);
        }
        else
        {
            this->connectionStatus = -1;
            HandlRegFiles(filePath + "index.html");
        }
    }
    //Location case block
    else if (this->withLocation == true)
    {
        
        if (locationBlock && locationBlock->isIndex == true)
            handleIndexFiles(locationBlock->Indexes, filePath);        /*Passing The indxing List of The Locatio Block*/
        else if (locationBlock->isIndex == false
                    && this->ServerConf.isIndexesSet == true)
        {
            handleIndexFiles(this->ServerConf.Indexes, filePath);  /*Passing The indxing List of The root Path*/
        }
        else if (locationBlock && locationBlock->isIndex == false
                    && this->ServerConf.isIndexesSet == false)
        {
			autoIndexingServing(this->ServerConf.autoindex, filePath);
        }
    }
}

/*Thid Methode Return The Position of The Closest Matching Prefix 
    To The requested Path*/
int     ClientRequest::getClosestMatchingPrefix(std::vector<LocationConfig>    locations) 
{
    std::string requested_path = this->clinentInfos.clientSourceReq;
    std::string location_path;
    int     bestMatchPos = -1;
    int     pos = 0;
    int     longMathcLenght = -1;
    for (std::vector<LocationConfig>::const_iterator it = locations.begin() ; it != locations.end() ; it++)
    {
       location_path = it->path;
       if (location_path == requested_path)
        {   
            this->clinentInfos.LocalisationName = it->path;
            return (pos);
        }
        if(requested_path.rfind(location_path, 0) == 0) // The location Path mmatch
        {
            if(static_cast<int>(location_path.length()) > longMathcLenght)
            {
                longMathcLenght = location_path.length();
                bestMatchPos = pos;
                this->clinentInfos.LocalisationName = it->path;
            }
        }
        pos++;
    }
    return (bestMatchPos);
}

/*This Methode Return The state of a PATH
    -1 : if not founded ;
    1 : if it is a file;    
    2 : if it is a DIR;    
*/
int    ClientRequest::getPathStatus(const  std::string&    pathname)
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

/*This Methode return The valid Root Path To work With*/
std::string     ClientRequest::getRoorPath(const	LocationConfig* 	ClientLocation) const
{
    std::string rootPath;
    //check which root to use 
    if(ClientLocation && ClientLocation->isRoot == true)
        rootPath = ClientLocation->root;
    else
        rootPath = this->ServerConf.rootPath;
    return (rootPath);
}


/*Globale case two : The Methode handle THe case when a location Match With THe PATH*/
void                    ClientRequest::NoMachingLocationHandler(const LocationConfig* locationBlock)
{
    std::string     toServeUrl;
    int             file_type;

    if (this->ServerConf.isRootSet == false)
    {
        noRootExisted(this->clinentInfos.clientSourceReq);
        return ;
    }
    toServeUrl = this->ServerConf.rootPath + this->clinentInfos.clientSourceReq;

    file_type = getPathStatus(toServeUrl);
    if (file_type == NOTFOUND)
    {
        this->connectionStatus = 0;
        HandlErr    noPermession(this, 404);
    }
    else if (file_type == NOTVALID)
        HandlErr    noPermession(this, 403);
    else if (file_type == ISREG)
        HandlRegFiles(toServeUrl);
    else
    {
        HandlDirectories(toServeUrl, locationBlock);
    }
}

/*Globale c
ase one : The Methode handle THe case when No location Match With THe PATH*/
void                    ClientRequest::MachingLocationHandler(const LocationConfig* locationBlock)
{
    std::string             toServeUrl;
    int                     file_type;
    std::string             rootPath;

    // Check if the HTTP method is allowed for this location
    if (!isMethodAllowedForLocation(locationBlock))
    {
        methodNotAllowedForLocation();
        return;
    }

    if (locationBlock && this->ServerConf.isRootSet == false && locationBlock->isRoot == false)
    {
        this->connectionStatus = -1;
        noRootExisted(this->clinentInfos.clientSourceReq);
    }
    rootPath = getRoorPath(locationBlock);
    toServeUrl = rootPath + this->clinentInfos.clientSourceReq;
    std::cout << RED << toServeUrl << RESET << std::endl;
    file_type = getPathStatus(toServeUrl);
    if (file_type == NOTFOUND)
    {  
        HandlErr    noPermession(this, 404);
    }
    else if (file_type == NOTVALID)
        HandlErr    noPermession(this, 403);
    else if (file_type == ISREG)
        HandlRegFiles(toServeUrl);
    else
    {
        HandlDirectories(toServeUrl, locationBlock);
    }
}

/*  Mathode to execute */
/*when the requested Methode is Get 
    This function take control*/
void                    ClientRequest::GetMethodeExec()
{
    int matchingPos;
    matchingPos = getClosestMatchingPrefix(this->ServerConf.locations);
    if(matchingPos == -1)
    {
        this->withLocation = false;
        NoMachingLocationHandler(NULL);
    }
    else
    {
        this->withLocation = true;
        const   LocationConfig* locationBlock =  &this->ServerConf.locations[matchingPos];
        MachingLocationHandler(locationBlock);
    }
    return ;
}