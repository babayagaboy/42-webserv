#!/usr/bin/env python3

from pathlib import Path
import sys
import json

p = Path(__file__).resolve().parent.parent / "files"

data = sys.stdin.read()

try:
    request = json.loads(data)
except json.JSONDecodeError:
    print("Status: 400 Bad Request")
    print("Content-Type: text/plain")
    print()
    print("Invalid JSON")
    sys.exit(0)

filename = request.get("filename")
content = request.get("content")

if not filename or content is None:
    print("Status: 400 Bad Request")
    print("Content-Type: text/plain")
    print()
    print("Missing filename or content")
    sys.exit(0)

file_path = p / filename

if file_path.exists():
    print("Status: 409 Conflict")
    print("Content-Type: text/plain")
    print()
    print("File already exists")
    sys.exit(0)

try:
    with file_path.open("w") as file:
        file.write(content)

    print("Status: 201 Created")
    print("Content-Type: text/plain")
    print()
    print("File posted successfully")

except OSError as e:
    print("Status: 500 Internal Server Error")
    print("Content-Type: text/plain")
    print()
    print(f"Could not create file: {e}")