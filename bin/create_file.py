#!/usr/bin/env python3

from pathlib import Path
import sys
import json

p = Path("files")

data = sys.stdin.read()
request = json.loads(data)

filename = request["filename"]
content = request["content"]

file_path = p / filename

if file_path.exists():
	print("HTTP/1.1 409 Conflic\r")
	print("Content-Type: application/json\r")
	print("\n\r")
	print(json.dumps({"status": "error", "message": "File already exists"}))

else:
	with file_path.open("w") as file:
		file.write(content)
	print("HTTP/1.1 201 Created\r")
	print("Content-Type: application/json\r")
	print("\n\r")
	print(json.dumps({"status": "ok", "message": "File created"}))