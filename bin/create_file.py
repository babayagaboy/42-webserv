#!/usr/bin/env python3

from pathlib import Path
import sys
import json

print("READING NOW")

p = Path("files")

data = sys.stdin.read()
request = json.loads(data)

filename = request["filename"]
content = request["content"]

file_path = p / filename

with file_path.open("w") as file:
    file.write(content)

print("From " + __file__ + " File created successfully: " + filename)