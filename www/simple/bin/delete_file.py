#!/usr/bin/env python3

from pathlib import Path
import sys
import json

try:
    data = sys.stdin.read()
    request = json.loads(data)

    filename = Path(request["filename"]).name
    file_path = Path(__file__).resolve().parent.parent / "files" / filename

    if not file_path.exists():
        print("Status: 404 Not Found")
        print("Content-Type: text/plain")
        print()
        print("File not found")
        sys.exit(0)

    file_path.unlink()

    print("Status: 200 OK")
    print("Content-Type: text/plain")
    print()
    print("File deleted successfully")

except (json.JSONDecodeError, KeyError):
    print("Status: 400 Bad Request")
    print("Content-Type: text/plain")
    print()
    print("Invalid DELETE request")

except OSError:
    print("Status: 500 Internal Server Error")
    print("Content-Type: text/plain")
    print()
    print("Could not delete file")
