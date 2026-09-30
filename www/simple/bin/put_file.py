#!/usr/bin/env python3

from pathlib import Path
import sys
import json

try:
    data = sys.stdin.read()
    request = json.loads(data)

    filename = Path(request["filename"]).name
    content = request["content"]

    if not isinstance(content, str):
        raise TypeError

    file_path = Path(__file__).resolve().parent.parent / "files" / filename

    file_path.write_text(content, encoding="utf-8")

    print("Status: 200 OK")
    print("Content-Type: text/plain")
    print()
    print("File updated successfully: " + filename)

except (json.JSONDecodeError, KeyError, TypeError):
    print("Status: 400 Bad Request")
    print("Content-Type: text/plain")
    print()
    print("Invalid PUT request")

except OSError:
    print("Status: 500 Internal Server Error")
    print("Content-Type: text/plain")
    print()
    print("Could not update file")