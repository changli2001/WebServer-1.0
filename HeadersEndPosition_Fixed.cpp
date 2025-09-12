/*
 * CORRECTED VERSION of HeadersEndPosition function
 * 
 * This function properly finds the end of HTTP headers by looking for "\r\n\r\n"
 * which marks the separation between headers and body in HTTP protocol.
 */

// Original function (with bug):
int HeadersEndPosition_Original(std::string Request)
{
    int firstCRLF;
    int SecondCRLF;

    firstCRLF = Request.find(CRLF);
    if(firstCRLF != -1)
    {
        SecondCRLF = Request.find(CRLF, firstCRLF + 2);
        if(SecondCRLF != -1)
        {
            return (firstCRLF);  // BUG: Returns position of first CRLF, not headers end
        }
    }
    return (-1);
}

// CORRECTED VERSION 1: Simple and direct
int HeadersEndPosition_Fixed(std::string Request)
{
    size_t pos = Request.find("\r\n\r\n");
    if (pos != std::string::npos) {
        return static_cast<int>(pos);  // Return start position of "\r\n\r\n"
    }
    return -1;  // Headers end not found
}

// CORRECTED VERSION 2: More explicit (step by step)
int HeadersEndPosition_Explicit(std::string Request)
{
    size_t pos = 0;
    
    // Look for the pattern "\r\n\r\n" which marks end of headers
    while (pos < Request.length()) {
        pos = Request.find("\r\n", pos);
        
        if (pos == std::string::npos) {
            return -1;  // No more CRLF found
        }
        
        // Check if we have another CRLF right after this one
        if (pos + 2 < Request.length() && 
            Request.substr(pos + 2, 2) == "\r\n") {
            return static_cast<int>(pos);  // Found "\r\n\r\n"
        }
        
        pos += 2;  // Move past this CRLF and continue searching
    }
    
    return -1;  // Headers end not found
}

// CORRECTED VERSION 3: What the original function probably intended
int HeadersEndPosition_Intended(std::string Request)
{
    int firstCRLF;
    int secondCRLF;

    firstCRLF = Request.find(CRLF);
    if(firstCRLF != -1)
    {
        secondCRLF = Request.find(CRLF, firstCRLF + 2);
        if(secondCRLF != -1)
        {
            // Check if these two CRLFs are consecutive (forming \r\n\r\n)
            if (secondCRLF == firstCRLF + 2) {
                return firstCRLF;  // Return start of "\r\n\r\n"
            }
        }
    }
    return -1;
}

/*
 * RECOMMENDATION: Use HeadersEndPosition_Fixed() as it's the simplest and most reliable.
 * 
 * Usage example:
 * 
 * std::string request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\nBody content";
 * int headers_end = HeadersEndPosition_Fixed(request);
 * 
 * if (headers_end != -1) {
 *     std::string headers = request.substr(0, headers_end);
 *     std::string body = request.substr(headers_end + 4);  // Skip "\r\n\r\n"
 *     // Process headers and body...
 * }
 */
