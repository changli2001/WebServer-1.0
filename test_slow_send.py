#!/usr/bin/env python3

import socket
import time
import sys

def send_slow(host, port, message):
    """Send message one byte at a time with 3-second delays"""
    try:
        # Create socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        
        # Connect to server
        print(f"Connecting to {host}:{port}")
        sock.connect((host, port))
        print("Connected successfully!")
        
        # Send message one byte at a time
        for i, byte in enumerate(message.encode('utf-8')):
            print(f"Sending byte {i+1}/{len(message)}: '{chr(byte)}' (0x{byte:02x})")
            sock.send(bytes([byte]))
            
            if i < len(message) - 1:  # Don't wait after last byte
                print("Waiting 3 seconds...")
                time.sleep(3)
        
        print("\nAll bytes sent! Waiting for response...")
        
        # Wait for response (with timeout)
        sock.settimeout(10)
        try:
            response = sock.recv(4096)
            print(f"Received response ({len(response)} bytes):")
            print(response.decode('utf-8', errors='ignore'))
        except socket.timeout:
            print("No response received within timeout")
        
    except Exception as e:
        print(f"Error: {e}")
    finally:
        sock.close()
        print("Connection closed")

if __name__ == "__main__":
    # Default values
    host = "localhost"
    port = 9090
    message = "Hello World!\n"
    
    # Parse command line arguments
    if len(sys.argv) > 1:
        host = sys.argv[1]
    if len(sys.argv) > 2:
        port = int(sys.argv[2])
    if len(sys.argv) > 3:
        message = sys.argv[3] + "\n"
    
    print(f"Will send: '{message.strip()}'")
    print(f"Target: {host}:{port}")
    print(f"Delay: 3 seconds between each byte")
    print("-" * 40)
    
    send_slow(host, port, message)
