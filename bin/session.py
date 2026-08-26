#!/usr/bin/env python3

import os

cookie = os.environ.get("HTTP_COOKIE", "")
session_id = "No session"

if cookie:
    cookies = cookie.split(";")

    for item in cookies:
        item = item.strip()

        if item.startswith("SessionId="):
            session_id = item[len("SessionId="):]
            break

print("Content-Type: text/html")
print()

print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("<title>Webserv Session Test</title>")
print("</head>")
print("<body>")

print("<h1>Webserv Session Test</h1>")

print("<p><strong>Session ID:</strong> " + session_id + "</p>")

print("<p>")
print("Refresh this page or make another request.")
print("</p>")

print("</body>")
print("</html>")