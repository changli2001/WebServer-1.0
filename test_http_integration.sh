#!/bin/bash

echo "=== Testing HTTP Request/Response Integration ==="
echo ""

# Check if webserver binary exists
if [ ! -f "./webserve" ]; then
    echo "Error: webserve binary not found. Please compile first with 'make'"
    exit 1
fi

# Check if config file exists
if [ ! -f "config/default.conf" ]; then
    echo "Error: config/default.conf not found"
    exit 1
fi

echo "1. Starting server in background..."
./webserve config/default.conf &
SERVER_PID=$!
echo "   Server PID: $SERVER_PID"

# Give server time to start
sleep 2

echo ""
echo "2. Testing HTTP GET request to root path..."
echo "   Sending: GET / HTTP/1.1"

# Test with curl to see the full HTTP response
echo "Response from server:"
echo "====================="
curl -v http://localhost:8080/ 2>&1 | head -20

echo ""
echo ""
echo "3. Testing static file serving..."
echo "   Sending: GET /index.html HTTP/1.1"

echo "Response from server:"
echo "====================="
curl -v http://localhost:8080/index.html 2>&1 | head -20

echo ""
echo ""
echo "4. Testing directory redirect..."
echo "   Sending: GET /error HTTP/1.1"

echo "Response from server:"
echo "====================="
curl -v http://localhost:8080/error 2>&1 | head -20

echo ""
echo ""
echo "5. Testing HTTP GET request to non-existent path..."
echo "   Sending: GET /nonexistent HTTP/1.1"

echo "Response from server:"
echo "====================="
curl -v http://localhost:8080/nonexistent 2>&1 | head -20

echo ""
echo ""
echo "6. Testing unsupported HTTP method..."
echo "   Sending: PUT / HTTP/1.1"

echo "Response from server:"
echo "====================="
curl -X PUT -v http://localhost:8080/ 2>&1 | head -20

echo ""
echo ""
echo "7. Cleaning up..."
kill $SERVER_PID 2>/dev/null

echo ""
echo "=== Test Complete ==="
echo "Check above for:"
echo "  - HTTP/1.1 200 OK response with webserv welcome page"
echo "  - Static file serving from website directory"
echo "  - HTTP/1.1 301 redirects for directories"
echo "  - HTTP/1.1 404 Not Found for non-existent paths"
echo "  - HTTP/1.1 405 Method Not Allowed for unsupported methods"
