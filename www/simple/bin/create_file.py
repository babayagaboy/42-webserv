#!/usr/bin/env python3

from pathlib import Path
import sys
import json
import os
from urllib.parse import parse_qs

p = Path(__file__).resolve().parent.parent / "files"

data = sys.stdin.read()

try:
    content_type = os.environ.get("CONTENT_TYPE", "")

    if content_type.startswith("application/x-www-form-urlencoded"):
        fields = parse_qs(data, keep_blank_values=True)
        request = {
            "filename": fields.get("filename", [""])[0],
            "content": fields.get("content", [""])[0],
        }
    else:
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

if not isinstance(filename, str) or Path(filename).name != filename or filename in (".", ".."):
    print("Status: 400 Bad Request")
    print("Content-Type: text/plain")
    print()
    print("Use a filename without folders")
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
    print("File successfully created.")

except OSError as e:
    print("Status: 500 Internal Server Error")
    print("Content-Type: text/plain")
    print()
    print(f"Could not create file: {e}")