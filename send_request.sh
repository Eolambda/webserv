#!/bin/bash
# send_request.sh
# This script sends a simple GET request to the specified server and port using netcat.

# Define the target server and port.
SERVER="localhost"
PORT=8080

# Define the HTTP GET request.
# The request consists of a request line and required headers, ending with an empty line.
REQUEST="GET / HTTP/1.1\r
Host: $SERVER\r
Connection: close\r
Cookie: username=admin;\r
\r\n"

# Send the request using netcat and display the response.
echo -e "$REQUEST" | nc $SERVER $PORT
