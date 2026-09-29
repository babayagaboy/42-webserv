#!/usr/bin/env python3

from pathlib import Path
import sys
import json

data = sys.stdin.read()
request = json.loads(data)

filename = Path(request["filename"]).name

file_path = Path(__file__).resolve().parent.parent / "files" / filename

if file_path.exists():
	file_path.unlink()

	print("Content-Type: text/plain")
	print()
	print("File deleted successfully")

else:
	print("Content-Type: text/plain")
	print()
	print("Error deleting file")