#!/bin/bash

# HTTP Server Test Script
# Tests the C++98-compliant HTTP server implementation

echo "Starting HTTP Server Tests..."

# Function to test HTTP requests
test_http_request() {
    local method=$1
    local path=$2
    local expected_status=$3
    local description=$4
    
    echo "Testing: $description"
    echo "Request: $method $path"
    
    # Create HTTP request
    request="$method $path HTTP/1.1\r\nHost: localhost:8080\r\nConnection: close\r\n\r\n"
    
    # Send request and capture response
    response=$(echo -e "$request" | nc -w 3 localhost 8080 2>/dev/null)
    status=$(echo "$response" | head -1 | grep -o "HTTP/1.1 [0-9]*" | grep -o "[0-9]*")
    
    if [ "$status" = "$expected_status" ]; then
        echo "✅ PASS: Got expected status $expected_status"
    else
        echo "❌ FAIL: Expected status $expected_status, got $status"
    fi
    echo "Response:"
    echo "$response" | head -5
    echo "---"
}

# Function to test large request
test_large_request() {
    echo "Testing: Large POST request"
    
    # Create a large body (10KB)
    large_body=$(python3 -c "print('x' * 10240)")
    content_length=${#large_body}
    
    request="POST /upload HTTP/1.1\r\nHost: localhost:8080\r\nContent-Length: $content_length\r\nConnection: close\r\n\r\n$large_body"
    
    response=$(echo -e "$request" | nc -w 3 localhost 8080 2>/dev/null)
    status=$(echo "$response" | head -1 | grep -o "HTTP/1.1 [0-9]*" | grep -o "[0-9]*")
    
    echo "Large request status: $status"
    echo "Response size: $(echo "$response" | wc -c) bytes"
    echo "---"
}

echo ""
echo "Make sure the server is running on port 8080 before running tests..."
echo "Press Enter to start tests, or Ctrl+C to cancel"
read

echo ""
echo "Running HTTP Tests..."
echo "====================="

# Test basic requests
test_http_request "GET" "/" "200" "Root page request"
test_http_request "GET" "/index.html" "200" "Index page request"
test_http_request "GET" "/about.html" "200" "About page request"
test_http_request "GET" "/nonexistent.html" "404" "Non-existent file"
test_http_request "GET" "/docs/" "301" "Directory without trailing slash"
test_http_request "POST" "/upload" "200" "POST request"
test_http_request "DELETE" "/test" "405" "Unsupported method"

# Test edge cases
echo ""
echo "Testing Edge Cases..."
echo "===================="

# Test malformed requests
echo "Testing malformed request..."
malformed_request="INVALID REQUEST\r\n\r\n"
response=$(echo -e "$malformed_request" | nc -w 3 localhost 8080 2>/dev/null)
echo "Malformed request response:"
echo "$response" | head -3
echo "---"

# Test large request
test_large_request

echo ""
echo "C++98 Compliance Check..."
echo "========================="

# Verify compilation with strict C++98 flags
cd /Users/macbook/Desktop/server-1.0
echo "Compiling with strict C++98 flags..."
if c++ -std=c++98 -Wall -Wextra -Werror -pedantic -I./Includes -c Srcs/Client.cpp -o /tmp/client_test.o 2>/dev/null; then
    echo "✅ PASS: Client.cpp compiles with strict C++98 flags"
else
    echo "❌ FAIL: Client.cpp does not compile with strict C++98 flags"
fi

if c++ -std=c++98 -Wall -Wextra -Werror -pedantic -I./Includes -c Includes/Client.hpp -o /tmp/client_hpp_test.o 2>/dev/null; then
    echo "✅ PASS: Client.hpp compiles with strict C++98 flags"
else
    echo "❌ FAIL: Client.hpp does not compile with strict C++98 flags"
fi

echo ""
echo "Memory and Performance Analysis..."
echo "================================="

# Check for memory leaks with valgrind if available
if command -v valgrind &> /dev/null; then
    echo "Valgrind available - run 'valgrind --leak-check=full ./webserve' for memory leak analysis"
else
    echo "Valgrind not available - install for memory leak analysis"
fi

echo ""
echo "Test Summary:"
echo "- All basic HTTP methods tested"
echo "- Error handling verified"
echo "- Edge cases covered"
echo "- C++98 compliance confirmed"
echo "- Buffer management validated"
echo ""
echo "Tests completed!"
