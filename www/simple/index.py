#!/usr/bin/env python3
import random
import datetime

print("Content-Type: text/html; charset=UTF-8")
print()  # Mandatory blank line between headers and body

now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
rand_num = random.randint(1, 100)

print(f"""<!DOCTYPE html>
<html>
<head><title>Python CGI Test</title></head>
<body>
    <h1>Python Dynamic Test</h1>
    <p>Current Time: <strong>{now}</strong></p>
    <p>Random Number: <strong>{rand_num}</strong></p>
</body>
</html>""")