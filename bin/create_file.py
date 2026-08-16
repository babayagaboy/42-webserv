#!/usr/bin/env python3

import sys
import json

print("READING NOW")

data = sys.stdin.read()

request = json.loads(data)

filename = request["filename"]
content = request["content"]

with open(filename, "w") as file:
    file.write(content)

print("Content-Type: text/plain")
print()
print("File created successfully: " + filename)