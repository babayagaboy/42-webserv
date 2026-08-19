#!/usr/bin/env python3

from pathlib import Path
import sys
import json


p = Path("files")

try:
    data = sys.stdin.read()
    request = json.loads(data)

    filename = request["filename"]
    changes = request["changes"]

    file_path = p / filename

    # Read existing JSON file
    with file_path.open("r") as file:
        content = json.load(file)

    # Apply partial modifications
    for key, value in changes.items():
        content[key] = value

    # Write modified JSON back
    with file_path.open("w") as file:
        json.dump(content, file, indent=4)

    print("Content-Type: text/plain")
    print()
    print("File patched successfully: " + filename)

except FileNotFoundError:
    print("Content-Type: text/plain")
    print()
    print("File not found")

except (json.JSONDecodeError, KeyError):
    print("Content-Type: text/plain")
    print()
    print("Invalid PATCH request")