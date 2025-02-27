#!/bin/bash
# send_request.sh
# This script sends a simple GET request to the specified server and port using netcat.

# Define the target server and port.
SERVER="localhost"
PORT=8080

# --- BASIC GET REQUEST ---
# REQUEST="GET /uploads/ HTTP/1.1\r
# Host: $SERVER\r
# Connection: close\r
# Cookie: username=admin;\r
# \r\n"

# --- CGI GET REQUEST ---
REQUEST="GET /test_route/index.html HTTP/1.1\r
Host: $SERVER\r
Connection: close\r
\r\n"

# --- BASIC DELETE REQUEST ---
# REQUEST="DELETE /delete_testing/abc HTTP/1.1\r
# Host: $SERVER\r
# Connection: close\r
# Cookie: username=admin;\r
# \r\n"

# Send the request using netcat and display the response.
echo -e "$REQUEST" | nc $SERVER $PORT
