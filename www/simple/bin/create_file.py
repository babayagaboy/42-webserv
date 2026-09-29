#!/usr/bin/env python3

from pathlib import Path
import sys
import json

p = Path("files")

data = sys.stdin.read()
request = json.loads(data)

filename = request["filename"]
content = request["content"]

file_path = p / filename

if file_path.exists() or not data:
    print("Status: 400 Bad Request")
    print("Content-Type: text/plain")
    print()
    print("Error posting file")

else:
    with file_path.open("w") as file:
        file.write(content)

    print("Status: 201 Created")
    print("Content-Type: text/plain")
    print()
    print("File posted successfully")