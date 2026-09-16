#!/usr/bin/env python3

from pathlib import Path
import sys
import json


p = Path("files")

try:
    request = json.loads(sys.stdin.read())
    filename = Path(request["filename"]).name
    changes = request["changes"]
    file_path = p / filename

    if not file_path.exists():
        print("Content-Type: text/plain")
        print()
        print("File not found")
        sys.exit(0)

    if not isinstance(changes, dict):
        raise TypeError

    with file_path.open("r", encoding="utf-8") as file:
        content = json.load(file)

    if not isinstance(content, dict):
        raise TypeError

    content.update(changes)

    with file_path.open("w", encoding="utf-8") as file:
        json.dump(content, file, indent=4)
        file.write("\n")

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