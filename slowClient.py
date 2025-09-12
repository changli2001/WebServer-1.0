import socket
import time
import sys

# Configuration
SERVER_HOST = 'localhost'
SERVER_PORT = 9090
MESSAGE = "Hello from the super slow client! This is a long message to test flow control.\n"
BYTES_PER_SECOND = 1/3  # Send 1 byte every 3 seconds

def main():
    # Create a socket and connect
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((SERVER_HOST, SERVER_PORT))
    print(f"Connected to {SERVER_HOST}:{SERVER_PORT}")
    
    # Send the message one byte at a time, with a delay
    message_bytes = MESSAGE.encode('utf-8')
    total_bytes = len(message_bytes)
    
    print(f"Sending {total_bytes} bytes, 1 byte every 3 seconds...")
    for i, byte in enumerate(message_bytes):
        # Create a single-byte chunk
        chunk = bytes([byte])
        try:
            bytes_sent = sock.send(chunk)
            print(f"Sent byte {i+1}/{total_bytes}: {chunk}")
        except Exception as e:
            print(f"Error sending: {e}")
            break
        
        # Wait before sending the next byte
        time.sleep(1 / BYTES_PER_SECOND) 
    
    print("Finished sending. Now reading response...")
    
    # Read the echo back from the server (you might want to make this slow too!)
    response = b''
    try:
        while True:
            chunk = sock.recv(1) # Read one byte at a time
            if not chunk:
                break
            response += chunk
            print(f"Received: {chunk.decode('utf-8', 'ignore')}")
            # time.sleep(0.5) # Uncomment to also slow down receiving
    except Exception as e:
        print(f"Error receiving: {e}")
    
    print(f"Full response: {response.decode('utf-8')}")
    sock.close()

if __name__ == "__main__":
    main()