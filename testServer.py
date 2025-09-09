#!/usr/bin/env python3

import socket
import time
import threading

# Configuration - EDIT THESE TO MATCH YOUR SERVER
SERVER_HOST = 'localhost'
SERVER_PORTS = [8080, 8081]  # Ports your server is listening on
MESSAGE = "HelloWorldThisIsATestMessageForTheEchoServer!"
SLOW_MESSAGE = "SlowAndSteadyWinsTheRace."
NUM_FAST_CLIENTS = 3
NUM_SLOW_CLIENTS = 2
BYTES_PER_SECOND = 0.33  # Send 1 byte every ~3 seconds

def test_fast_client(port, client_id, message):
    """A client that sends its entire message at once."""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.connect((SERVER_HOST, port))
            print(f"[Fast Client {client_id} on port {port}] Connected. Sending: '{message}'")
            sock.sendall(message.encode('utf-8'))
            print(f"[Fast Client {client_id} on port {port}] Message sent. Waiting for echo...")
            
            # Receive the echo
            response = b''
            while len(response) < len(message):
                chunk = sock.recv(1024)
                if not chunk:
                    break
                response += chunk
            response = response.decode('utf-8')
            
            # Verify the echo
            if response == message:
                print(f"[Fast Client {client_id} on port {port}] ✅ SUCCESS: Echo correct.")
            else:
                print(f"[Fast Client {client_id} on port {port}] ❌ FAILURE: Echo mismatch! Sent: '{message}', Received: '{response}'")
                
    except Exception as e:
        print(f"[Fast Client {client_id} on port {port}] ❌ ERROR: {e}")

def test_slow_client(port, client_id, message, delay):
    """A client that sends its message one byte at a time with a delay."""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.connect((SERVER_HOST, port))
            print(f"[Slow Client {client_id} on port {port}] Connected. Sending '{message}' slowly.")
            
            message_bytes = message.encode('utf-8')
            total_bytes = len(message_bytes)
            
            # Send slowly
            for i, byte in enumerate(message_bytes):
                chunk = bytes([byte])
                sock.sendall(chunk)
                # print(f"[Slow Client {client_id}] Sent byte {i+1}/{total_bytes}: {chunk}")
                time.sleep(delay)
            
            print(f"[Slow Client {client_id} on port {port}] Finished sending. Waiting for echo...")
            
            # Receive the echo (might also come slowly)
            response = b''
            while len(response) < len(message):
                chunk = sock.recv(1) # Try to read byte by byte
                if not chunk:
                    break
                response += chunk
                # print(f"[Slow Client {client_id}] Received: {chunk}")
                # Optionally, also slow down the receiving
                # time.sleep(delay)
            
            response = response.decode('utf-8')
            
            # Verify the echo
            if response == message:
                print(f"[Slow Client {client_id} on port {port}] ✅ SUCCESS: Echo correct.")
            else:
                print(f"[Slow Client {client_id} on port {port}] ❌ FAILURE: Echo mismatch! Sent: '{message}', Received: '{response}' (Length: {len(response)})")
                
    except Exception as e:
        print(f"[Slow Client {client_id} on port {port}] ❌ ERROR: {e}")

def main():
    print(f"Starting comprehensive test of echo server on {SERVER_HOST}:{SERVER_PORTS}")
    print("=" * 60)
    threads = []

    # Test each port configured in the server
    for port in SERVER_PORTS:
        print(f"\n--- Testing Port {port} ---")
        
        # Create fast clients for this port
        for i in range(NUM_FAST_CLIENTS):
            thread = threading.Thread(
                target=test_fast_client,
                args=(port, i, f"{MESSAGE} (Client{i} Port{port})")
            )
            threads.append(thread)
            thread.start()
            time.sleep(0.1) # Small delay to space out connections

        # Create slow clients for this port
        for i in range(NUM_SLOW_CLIENTS):
            thread = threading.Thread(
                target=test_slow_client,
                args=(port, i, f"{SLOW_MESSAGE} (SlowClient{i} Port{port})", 1.0/BYTES_PER_SECOND)
            )
            threads.append(thread)
            thread.start()
            time.sleep(0.5) # Bigger delay for slow clients

    # Wait for all client threads to finish
    print(f"\n[Main] All clients started. Waiting for them to finish...")
    for thread in threads:
        thread.join()

    print("\n" + "=" * 60)
    print("Stress test completed.")

if __name__ == "__main__":
    main()
