#!/usr/bin/env python3

from pathlib import Path
import sys
import json

print("PUTTING NOW")

p = Path("files")

data = sys.stdin.read()
request = json.loads(data)

filename = request["filename"]
content = request["content"]
file_path = Path("files") / filename

file_path = p / filename

if file_path.exists():
	with file_path.open("w") as file:
		file.write(content)
	print("From" + __file__ + " File edited successfully: " + filename)
else:
    print("File not found: " + filename)