#!/usr/bin/env python3

from pathlib import Path
import sys
import json

data = sys.stdin.read()
request = json.loads(data)

filename = request["filename"]
content = request["content"]

file_path = Path("files") / filename

with file_path.open("w") as file:
    file.write(content)

print("HTTP/1.1 200 OK\r")
print("Content-Type: text/plain")
print()
print("File updated successfully: " + filename)