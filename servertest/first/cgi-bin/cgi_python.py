#!/usr/bin/env python3
"""
A simple CGI script to test POST handling.
This script:
 - Reads the HTTP request method from the environment.
 - If the method is POST, it reads the CONTENT_LENGTH environment variable to determine
   how many bytes of data to read from standard input (stdin).
 - It then prints an HTTP response (with a Content-Type header) followed by an HTML page
   that echoes the received POST data.
 - If the request is not POST, it informs the client that only POST is supported.
"""

import os
import sys

# First, print the required HTTP header.
# The blank line after the headers is crucial to separate headers from the body.
print("Content-Type: text/html")
print()  # This prints a blank line

# Retrieve the HTTP request method from the environment.
# Default to "GET" if not specified, then convert it to uppercase.
method = os.environ.get("REQUEST_METHOD", "GET").upper()

# If the request is a POST...
if method == "POST":
    try:
        # Retrieve the CONTENT_LENGTH environment variable,
        # which tells us how many bytes to read from stdin.
        content_length = int(os.environ.get("CONTENT_LENGTH", "0"))
    except ValueError:
        content_length = 0

    # Read the POST data from standard input. This call will block until
    # the specified number of bytes have been read.
    post_data = sys.stdin.read(content_length)
    
    # Generate an HTML response that displays the received POST data.
    print("<html>")
    print("<head><title>POST CGI Test</title></head>")
    print("<body>")
    print("<h1>POST CGI Test</h1>")
    print("<p>Received POST data:</p>")
    # The <pre> tag is used to preserve formatting of the raw POST data.
    print("<pre>{}</pre>".format(post_data))
    print("</body>")
    print("</html>")

# If the request is not a POST...
else:
    # Inform the client that this script only handles POST requests.
    print("<html>")
    print("<head><title>CGI Test</title></head>")
    print("<body>")
    print("<h1>This CGI script only handles POST requests.</h1>")
    print("</body>")
    print("</html>")
    #boucle infinie :
    while True:
        pass
