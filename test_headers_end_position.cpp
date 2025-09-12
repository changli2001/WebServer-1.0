#include <iostream>
#include <string>
#include <cassert>
#include <vector>

// Include the CRLF definition
#define CRLF "\r\n"

// Copy of the function to test (with the current implementation)
int HeadersEndPosition(std::string Request)
{
    int firstCRLF;
    int SecondCRLF;

    firstCRLF = Request.find(CRLF);
    if(firstCRLF != -1)
    {
        SecondCRLF = Request.find(CRLF, firstCRLF + 2);
        if(SecondCRLF != -1)
        {
            return (firstCRLF);
        }
    }
    return (-1); // no CRLF founded;
}

// Corrected version for comparison
int HeadersEndPositionCorrect(std::string Request)
{
    size_t pos = Request.find("\r\n\r\n");
    if (pos != std::string::npos) {
        return static_cast<int>(pos);
    }
    return -1;
}

// Test structure
struct TestCase {
    std::string name;
    std::string input;
    int expected_original;     // Expected result from original function
    int expected_correct;      // Expected result from corrected function
    std::string description;
};

// Color codes for output
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define RESET   "\033[0m"

void runTest(const TestCase& test) {
    std::cout << BLUE << "Testing: " << test.name << RESET << std::endl;
    std::cout << "Description: " << test.description << std::endl;
    std::cout << "Input: \"" << test.input << "\"" << std::endl;
    
    // Test original function
    int result_original = HeadersEndPosition(test.input);
    std::cout << "Original function result: " << result_original << std::endl;
    std::cout << "Expected (original): " << test.expected_original << std::endl;
    
    // Test corrected function
    int result_correct = HeadersEndPositionCorrect(test.input);
    std::cout << "Corrected function result: " << result_correct << std::endl;
    std::cout << "Expected (correct): " << test.expected_correct << std::endl;
    
    // Check results
    bool original_pass = (result_original == test.expected_original);
    bool correct_pass = (result_correct == test.expected_correct);
    
    std::cout << "Original function: " << (original_pass ? GREEN "PASS" : RED "FAIL") << RESET << std::endl;
    std::cout << "Corrected function: " << (correct_pass ? GREEN "PASS" : RED "FAIL") << RESET << std::endl;
    
    std::cout << "-------------------------------------------" << std::endl;
}

int main() {
    std::cout << YELLOW << "=== HeadersEndPosition Function Unit Tests ===" << RESET << std::endl << std::endl;
    
    std::vector<TestCase> tests;
    
    // Test 1: Simple valid HTTP request
    TestCase test1 = {
        "Simple GET Request",
        "GET / HTTP/1.1\r\nHost: localhost\r\n\r\nBody content here",
        15, // Original function returns position of first CRLF (after "GET / HTTP/1.1")
        30, // Correct function returns position of "\r\n\r\n"
        "Basic HTTP GET request with headers ending in \\r\\n\\r\\n"
    };
    tests.push_back(test1);
    
    // Test 2: POST request with multiple headers
    TestCase test2 = {
        "POST Request with Headers",
        "POST /api HTTP/1.1\r\nHost: example.com\r\nContent-Type: application/json\r\n\r\n{\"data\":\"test\"}",
        18, // Position after "POST /api HTTP/1.1"
        73, // Position of actual headers end
        "POST request with multiple headers"
    };
    tests.push_back(test2);
    
    // Test 3: Request with no headers end
    TestCase test3 = {
        "Incomplete Headers",
        "GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 100\r\n",
        15, // Position after first line
        -1, // No double CRLF found
        "HTTP request without complete headers (missing \\r\\n\\r\\n)"
    };
    tests.push_back(test3);
    
    // Test 4: Empty string
    TestCase test4 = {
        "Empty String",
        "",
        -1, // No CRLF found
        -1, // No double CRLF found
        "Empty input string"
    };
    tests.push_back(test4);
    
    // Test 5: Only single CRLF
    TestCase test5 = {
        "Single CRLF Only",
        "GET / HTTP/1.1\r\n",
        -1, // Only one CRLF found, need two
        -1, // No double CRLF
        "Request with only one CRLF"
    };
    tests.push_back(test5);
    
    // Test 6: Headers with no body
    TestCase test6 = {
        "Headers Only",
        "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n",
        15, // Position after first line
        30, // Position of headers end
        "Complete headers with no body content"
    };
    tests.push_back(test6);
    
    // Run all tests
    int passed_original = 0;
    int passed_correct = 0;
    
    for (size_t i = 0; i < tests.size(); i++) {
        const TestCase& test = tests[i];
        runTest(test);
        
        // Count passes
        if (HeadersEndPosition(test.input) == test.expected_original) {
            passed_original++;
        }
        if (HeadersEndPositionCorrect(test.input) == test.expected_correct) {
            passed_correct++;
        }
    }
    
    // Summary
    std::cout << YELLOW << "=== Test Summary ===" << RESET << std::endl;
    std::cout << "Total tests: " << tests.size() << std::endl;
    std::cout << "Original function passed: " << GREEN << passed_original << "/" << tests.size() << RESET << std::endl;
    std::cout << "Corrected function passed: " << GREEN << passed_correct << "/" << tests.size() << RESET << std::endl;
    
    // Analysis
    std::cout << std::endl << YELLOW << "=== Analysis ===" << RESET << std::endl;
    std::cout << "The original function has a logic error:" << std::endl;
    std::cout << "- It returns the position of the FIRST \\r\\n, not the start of \\r\\n\\r\\n" << std::endl;
    std::cout << "- For HTTP headers, we need to find where \\r\\n\\r\\n starts" << std::endl;
    std::cout << "- The corrected version properly finds the end of headers marker" << std::endl;
    
    return 0;
}
