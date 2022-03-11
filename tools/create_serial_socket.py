#!/usr/bin/python3

import socket
import sys
import os

# This is the path that Zephyr expects for its serial interace
SOCKET_LOCATION: str = '/tmp/slip.sock'

# Number of bytes to process on each receive
RECV_BUFFER_SIZE: int = 1024

# Clean up from our last run
if os.path.exists(SOCKET_LOCATION):
     os.remove(SOCKET_LOCATION)

sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
sock.bind(SOCKET_LOCATION)
sock.listen(1)

while True:
     print("Waiting for a connection...")
     try:
          connection, client_addr = sock.accept()
          print(client_addr)
          print(f"Got connection! Client address: {str(client_addr)}!")
          while True:
               data = connection.recv(RECV_BUFFER_SIZE)
               print(f"Got raw bytes: {str(data)}")
               if not data:
                    break
     finally:
          connection.close()
