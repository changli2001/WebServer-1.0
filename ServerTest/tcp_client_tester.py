#!/usr/bin/env python3
"""
Real TCP Client Tester for HTTP Server
This script creates actual TCP connections to test your C++ server
"""

import socket
import threading
import time
import random
import string
import sys
import argparse
from typing import List, Dict
import json

class TCPClientTester:
    def __init__(self, host: str = "localhost"):
        self.host = host
        self.clients: List[Dict] = []
        self.stats = {
            'active_connections': 0,
            'total_bytes_sent': 0,
            'total_bytes_received': 0,
            'successful_connections': 0,
            'failed_connections': 0,
            'total_attempts': 0
        }
        self.running = False
        self.lock = threading.Lock()

    def generate_random_text(self, length: int) -> str:
        """Generate random text of specified length"""
        characters = string.ascii_letters + string.digits + ' .,!?-_'
        return ''.join(random.choice(characters) for _ in range(length))

    def log(self, message: str, client_id: int = None):
        """Thread-safe logging"""
        timestamp = time.strftime("%H:%M:%S")
        prefix = f"[{timestamp}]"
        if client_id is not None:
            prefix += f" [Client {client_id}]"
        print(f"{prefix} {message}")

    def update_stats(self, **kwargs):
        """Thread-safe stats update"""
        with self.lock:
            for key, value in kwargs.items():
                if key in self.stats:
                    self.stats[key] += value

    def client_worker(self, client_config: Dict):
        """Worker function for each client thread"""
        client_id = client_config['id']
        port = client_config['port']
        total_bytes = client_config['total_bytes']
        bytes_per_second = client_config['bytes_per_second']
        message_length = client_config['message_length']
        
        sock = None
        try:
            # Create socket and connect
            self.log(f"Connecting to {self.host}:{port}...", client_id)
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(10)  # 10 second timeout
            
            sock.connect((self.host, port))
            self.log(f"Connected successfully!", client_id)
            
            self.update_stats(active_connections=1, successful_connections=1)
            
            bytes_sent = 0
            bytes_received = 0
            
            # Calculate timing for rate limiting
            delay_between_sends = message_length / bytes_per_second if bytes_per_second > 0 else 0
            
            while bytes_sent < total_bytes and self.running:
                # Calculate how many bytes to send in this iteration
                remaining_bytes = total_bytes - bytes_sent
                bytes_to_send = min(message_length, remaining_bytes)
                
                # Generate random message
                message = self.generate_random_text(bytes_to_send)
                
                # Send message
                try:
                    sock.send(message.encode('utf-8'))
                    bytes_sent += len(message)
                    self.update_stats(total_bytes_sent=len(message))
                    
                    self.log(f"Sent {len(message)} bytes: '{message[:50]}{'...' if len(message) > 50 else ''}'", client_id)
                    
                    # Try to receive echo response
                    try:
                        sock.settimeout(2)  # Short timeout for receive
                        response = sock.recv(bytes_to_send)
                        if response:
                            bytes_received += len(response)
                            self.update_stats(total_bytes_received=len(response))
                            response_str = response.decode('utf-8', errors='ignore')
                            self.log(f"Received echo {len(response)} bytes: '{response_str[:50]}{'...' if len(response_str) > 50 else ''}'", client_id)
                    except socket.timeout:
                        self.log("No echo response received (timeout)", client_id)
                    except Exception as e:
                        self.log(f"Error receiving data: {e}", client_id)
                    
                    # Rate limiting
                    if delay_between_sends > 0:
                        time.sleep(delay_between_sends)
                        
                except Exception as e:
                    self.log(f"Error sending data: {e}", client_id)
                    break
            
            self.log(f"Completed! Sent: {bytes_sent} bytes, Received: {bytes_received} bytes", client_id)
            
        except Exception as e:
            self.log(f"Connection failed: {e}", client_id)
            self.update_stats(failed_connections=1)
        
        finally:
            if sock:
                try:
                    sock.close()
                    self.log("Disconnected", client_id)
                except:
                    pass
            self.update_stats(active_connections=-1)

    def run_test(self, client_configs: List[Dict]):
        """Run the test with multiple clients"""
        self.running = True
        self.stats = {
            'active_connections': 0,
            'total_bytes_sent': 0,
            'total_bytes_received': 0,
            'successful_connections': 0,
            'failed_connections': 0,
            'total_attempts': len(client_configs)
        }
        
        self.log(f"Starting test with {len(client_configs)} clients...")
        
        threads = []
        
        # Start all client threads
        for config in client_configs:
            thread = threading.Thread(target=self.client_worker, args=(config,))
            thread.daemon = True
            thread.start()
            threads.append(thread)
            time.sleep(0.1)  # Small delay between connections
        
        try:
            # Wait for all threads to complete or user interruption
            for thread in threads:
                thread.join()
        except KeyboardInterrupt:
            self.log("Test interrupted by user")
            self.running = False
        
        self.log("Test completed!")
        self.print_final_stats()

    def print_final_stats(self):
        """Print final statistics"""
        print("\n" + "="*50)
        print("FINAL TEST STATISTICS")
        print("="*50)
        print(f"Total Attempts: {self.stats['total_attempts']}")
        print(f"Successful Connections: {self.stats['successful_connections']}")
        print(f"Failed Connections: {self.stats['failed_connections']}")
        print(f"Success Rate: {(self.stats['successful_connections']/self.stats['total_attempts']*100):.1f}%")
        print(f"Total Bytes Sent: {self.format_bytes(self.stats['total_bytes_sent'])}")
        print(f"Total Bytes Received: {self.format_bytes(self.stats['total_bytes_received'])}")
        print("="*50)

    def format_bytes(self, bytes_val: int) -> str:
        """Format bytes in human readable format"""
        for unit in ['B', 'KB', 'MB', 'GB']:
            if bytes_val < 1024.0:
                return f"{bytes_val:.2f} {unit}"
            bytes_val /= 1024.0
        return f"{bytes_val:.2f} TB"


def main():
    parser = argparse.ArgumentParser(description='TCP Client Tester for HTTP Server')
    parser.add_argument('--host', default='localhost', help='Server host (default: localhost)')
    parser.add_argument('--config', help='JSON config file with client configurations')
    parser.add_argument('--interactive', action='store_true', help='Interactive mode')
    
    args = parser.parse_args()
    
    tester = TCPClientTester(args.host)
    
    if args.config:
        # Load configuration from JSON file
        try:
            with open(args.config, 'r') as f:
                client_configs = json.load(f)
        except Exception as e:
            print(f"Error loading config file: {e}")
            return
    elif args.interactive:
        # Interactive mode
        client_configs = []
        print("=== TCP Client Tester - Interactive Mode ===")
        
        try:
            num_clients = int(input("Number of clients: "))
            
            for i in range(1, num_clients + 1):
                print(f"\n--- Client {i} Configuration ---")
                port = int(input(f"Port for client {i} (default: {8080 + i - 1}): ") or (8080 + i - 1))
                total_bytes = int(input(f"Total bytes to send (default: {1024 * i}): ") or (1024 * i))
                bytes_per_second = int(input(f"Bytes per second (default: 512): ") or 512)
                message_length = int(input(f"Message length in bytes (default: 256): ") or 256)
                
                client_configs.append({
                    'id': i,
                    'port': port,
                    'total_bytes': total_bytes,
                    'bytes_per_second': bytes_per_second,
                    'message_length': message_length
                })
        except (ValueError, KeyboardInterrupt):
            print("Invalid input or interrupted")
            return
    else:
        # Default configuration
        client_configs = [
            {'id': 1, 'port': 8080, 'total_bytes': 1024, 'bytes_per_second': 512, 'message_length': 256},
            {'id': 2, 'port': 8081, 'total_bytes': 2048, 'bytes_per_second': 512, 'message_length': 256},
            {'id': 3, 'port': 8082, 'total_bytes': 4096, 'bytes_per_second': 512, 'message_length': 256},
        ]
        print("Using default configuration (3 clients on ports 8080-8082)")
    
    # Run the test
    try:
        tester.run_test(client_configs)
    except KeyboardInterrupt:
        print("\nTest interrupted by user")


if __name__ == "__main__":
    main()
