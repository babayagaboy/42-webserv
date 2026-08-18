#!/usr/bin/env python3

from pathlib import Path
import sys
import json

data = sys.stdin.read()
request = json.loads(data)

filename = request["filename"]

file_path = Path("files") / filename

if file_path.exists():
	file_path.unlink()

	print("HTTP/1.1 200 OK\r")
	print("Content-Type: application/json\r")
	print("\n\r")
	print(json.dumps({
		"status": "ok",
		"message": "File deleted successfully",
		"filename": filename
	}))
else:
	print("HTTP/1.1 404 Not Found\r")
	print("Content-Type: application/json\r")
	print("\r\n")
	print(json.dumps({
		"status": "error",
		"message": "File not found",
		"filename": filename
	}))