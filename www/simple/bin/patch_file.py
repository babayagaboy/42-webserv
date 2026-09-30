#!/usr/bin/env python3

from pathlib import Path
import sys
import json

p = Path(__file__).resolve().parent.parent / "files"

try:
    request = json.loads(sys.stdin.read())

    filename = Path(request["filename"]).name
    file_path = p / filename

    if not file_path.exists():
        print("Status: 404 Not Found")
        print("Content-Type: text/plain")
        print()
        print("File not found")
        sys.exit(0)

    if "content" in request:
        content = request["content"]

        if not isinstance(content, str):
            raise TypeError

        file_path.write_text(content, encoding="utf-8")

    elif "changes" in request:
        changes = request["changes"]

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

    else:
        print("Status: 400 Bad Request")
        print("Content-Type: text/plain")
        print()
        print("PATCH request must contain 'content' or 'changes'")
        sys.exit(0)

    print("Status: 200 OK")
    print("Content-Type: text/plain")
    print()
    print("File patched successfully: " + filename)

except (json.JSONDecodeError, KeyError, TypeError):
    print("Status: 400 Bad Request")
    print("Content-Type: text/plain")
    print()
    print("Invalid PATCH request")

except OSError:
    print("Status: 500 Internal Server Error")
    print("Content-Type: text/plain")
    print()
    print("Could not modify file")
