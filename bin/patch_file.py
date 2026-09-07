#!/usr/bin/env python3

from pathlib import Path
import sys
import json


p = Path("files")

try:
    data = sys.stdin.read()

    request = json.loads(data)

    filename = request["filename"]
    content = request["content"]

    file_path = p / filename

    if not file_path.exists():
        print("Content-Type: text/plain")
        print()
        print("File not found")
        sys.exit(0)

    with file_path.open("w") as file:
        file.write(content)

    print("Content-Type: text/plain")
    print()
    print("File patched successfully: " + filename)

except (json.JSONDecodeError, KeyError, TypeError):
    print("Content-Type: text/plain")
    print()
    print("Invalid PATCH request")

except OSError:
    print("Content-Type: text/plain")
    print()
    print("Could not modify file")