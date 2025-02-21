#!/usr/bin/env python3
"""
A basic Python CGI script that prints a simple HTML page and echoes
the CGI environment variables for testing purposes.
"""

import os

def main():
    # Print the required HTTP header for content type
    print("Content-Type: text/html")
    print()  # End of headers

    # Begin the HTML document
    print("<html>")
    print("<head>")
    print("  <title>Simple CGI Script Test</title>")
    print("</head>")
    print("<body>")
    print("  <h1>CGI Script Test</h1>")
    print("  <p>This is a simple Python CGI script running successfully.</p>")

    # Optionally, list the CGI environment variables for debugging
    print("  <h2>CGI Environment Variables</h2>")
    print("  <ul>")
    for key, value in os.environ.items():
        print(f"    <li><strong>{key}</strong>: {value}</li>")
    print("  </ul>")

    print("</body>")
    print("</html>")

if __name__ == '__main__':
    main()