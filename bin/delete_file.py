#!/usr/bin/env python3

from pathlib import Path
import sys
import json

print("DELETING NOW")

data = sys.stdin.read()
request = json.loads(data)

filename = request["filename"]

file_path = Path("files") / filename

if file_path.exists():
    file_path.unlink()
    print("From" + __file__ + " File deleted successfully: " + filename)
else:
    print("File not found: " + filename)