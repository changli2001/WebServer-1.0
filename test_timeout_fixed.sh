#!/bin/bash

echo "=== Testing Client Timeout Functionality ==="
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
echo "2. Testing connection messages..."
echo "   Connecting to server to see connection message..."

# Test connection with nc (netcat) - connect and immediately disconnect
echo "GET / HTTP/1.1" | nc localhost 8080 > /dev/null 2>&1 &
NC_PID=$!

sleep 1
kill $NC_PID 2>/dev/null

echo ""
echo "3. Testing timeout functionality..."
echo "   Connecting but staying idle for 35 seconds..."
echo "   (Client should timeout after 30 seconds)"

# Connect but stay idle - this should timeout
nc localhost 8080 &
NC_IDLE_PID=$!

echo "   Connected with PID: $NC_IDLE_PID"
echo "   Waiting 35 seconds to see timeout..."

# Wait and monitor server output
sleep 35

echo ""
echo "4. Cleaning up..."
kill $NC_IDLE_PID 2>/dev/null
kill $SERVER_PID 2>/dev/null

echo ""
echo "=== Test Complete ==="
echo "Check the server output above for:"
echo "  - Green 'New Client connected' messages"
echo "  - 'Select timeout - checking for client timeouts...' messages every 5 seconds"
echo "  - 'Client X (IP) timed out after 30 seconds' messages"
