#include <iostream>
#include <string>

#define CRLF "\r\n"

// Original function from your code
int HeadersEndPosition(std::string Request)
{
    int firstCRLF;
    int SecondCRLF;

    firstCRLF = Request.find(CRLF);
    if(firstCRLF != -1)
    {
        SecondCRLF = Request.find(CRLF, firstCRLF + 2);
        if(SecondCRLF != 0)
        {
            return (firstCRLF);
        }
    }
    return (-1);
}

// Fixed version
int HeadersEndPositionFixed(std::string Request)
{
    size_t pos = Request.find("\r\n\r\n");
    if (pos != std::string::npos) {
        return static_cast<int>(pos);
    }
    return -1;
}

void analyzeString(const std::string& request, const std::string& name) {
    std::cout << "=== " << name << " ===" << std::endl;
    std::cout << "String: \"" << request << "\"" << std::endl;
    std::cout << "Length: " << request.length() << std::endl;
    
    // Find all CRLF positions
    size_t pos = 0;
    int count = 0;
    while ((pos = request.find("\r\n", pos)) != std::string::npos) {
        std::cout << "CRLF #" << (++count) << " at position: " << pos << std::endl;
        pos += 2;
    }
    
    // Find double CRLF
    size_t double_crlf = request.find("\r\n\r\n");
    if (double_crlf != std::string::npos) {
        std::cout << "Double CRLF (\\r\\n\\r\\n) at position: " << double_crlf << std::endl;
    } else {
        std::cout << "No double CRLF found" << std::endl;
    }
    
    // Test functions
    int original_result = HeadersEndPosition(request);
    int fixed_result = HeadersEndPositionFixed(request);
    
    std::cout << "Original function result: " << original_result << std::endl;
    std::cout << "Fixed function result: " << fixed_result << std::endl;
    std::cout << std::endl;
}

int main() {
    // Test cases
    std::string test1 = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\nBody content here";
    std::string test2 = "POST /api HTTP/1.1\r\nHost: example.com\r\nContent-Type: application/json\r\n\r\n{\"data\":\"test\"}";
    std::string test3 = "GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 100\r\n";
    std::string test4 = "";
    std::string test5 = "GET / HTTP/1.1\r\n";
    std::string test6 = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    
    analyzeString(test1, "Simple GET Request");
    analyzeString(test2, "POST with Headers");
    analyzeString(test3, "Incomplete Headers");
    analyzeString(test4, "Empty String");
    analyzeString(test5, "Single CRLF");
    analyzeString(test6, "Headers Only");
    
    return 0;
}
