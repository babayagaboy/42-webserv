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

	print("Content-Type: text/plain")
	print()
	print("File deleted successfully")

else:
	print("Content-Type: text/plain")
	print()
	print("Error deleting file")