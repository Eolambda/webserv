#!/bin/bash
# send_request.sh
# This script sends either a simple GET or a POST request to the specified server and port using netcat.
# Usage:
#   ./send_request.sh GET
#   ./send_request.sh POST
# For POST, you can customize the body by setting the environment variable BODY.
# For example:
#   BODY="name=John+Doe&email=john@example.com" ./send_request.sh POST

# Define the target server and port.
SERVER="localhost"
PORT=8080

if [ "$1" == "POST" ]; then
    # Use the BODY environment variable if set, otherwise use a default.
    BODY="${BODY:-name=John+Doe&email=john@example.com}"
    
    # Calculate the content length of the POST body in bytes.
    CONTENT_LENGTH=$(echo -ne "$BODY" | wc -c)
    
    # Build the POST request.
    REQUEST="POST /uploads/ HTTP/1.1\r
Host: $SERVER\r
Content-Type: application/x-www-form-urlencoded\r
Content-Length: $CONTENT_LENGTH\r
Connection: close\r
\r
$BODY\r\n"
else
    # Default to a GET request.
    REQUEST="GET /uploads/test.py HTTP/1.1\r
Host: $SERVER\r
Connection: close\r
\r\n"
fi

# Display the request for debugging.
echo -e "Sending request:\n$REQUEST"

# Send the request using netcat and display the server's response.
echo -e "$REQUEST" | nc $SERVER $PORT
