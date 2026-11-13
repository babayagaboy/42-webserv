#!/usr/bin/env python3

"""Send local HTTP requests for the JavaScript-free forms."""

import http.client
import json
import os
import sys
from urllib.parse import parse_qs, quote

from list_files import render_file_list


def send_request(fields):
    method = fields.get("method", [""])[0]
    filename = fields.get("filename", [""])[0]
    content = fields.get("content", [""])[0]
    new_filename = fields.get("new_filename", [""])[0]

    if new_filename:
        if method != "PUT":
            raise ValueError("Choose a saved file for PATCH. New filenames are only for PUT.")
        if filename:
            raise ValueError("Choose a saved file or enter a new filename, not both.")
        filename = new_filename

    endpoints = {
        "DELETE": "/delete_file.py",
        "PUT": "/put_file.py",
        "PATCH": "/patch_file.py",
    }

    if method not in ("DELETE", "PUT", "PATCH", "HEAD", "OPTIONS"):
        raise ValueError("Choose a supported HTTP method.")

    if filename and (
        filename in (".", "..")
        or "/" in filename
        or "\\" in filename
        or any(ord(character) < 32 for character in filename)
    ):
        raise ValueError("Enter a filename without folders or control characters.")

    body = None
    headers = {}

    if method in endpoints:
        if not filename:
            raise ValueError("Enter the name of the file to change.")

        path = endpoints[method]
        body = json.dumps({"filename": filename, "content": content})
        headers["Content-Type"] = "application/json"
    else:
        path = "/"

        if filename:
            path = "/files/" + quote(filename, safe="")

    # Only contact this server, never a user-supplied hostname or URL.
    port = int(os.environ["SERVER_PORT"])
    connection = http.client.HTTPConnection("127.0.0.1", port, timeout=5)

    try:
        connection.request(method, path, body, headers)
        response = connection.getresponse()
        response_body = response.read().decode("utf-8", errors="replace")
        lines = [f"{method} {path}", f"Status: {response.status} {response.reason}", ""]

        if method in ("HEAD", "OPTIONS"):
            for name, value in response.getheaders():
                lines.append(f"{name}: {value}")
        elif response.getheader("Content-Type", "").startswith("text/html"):
            lines.append("The server returned an HTML error page.")
        else:
            lines.append(response_body)

        return response.status, "\n".join(lines)
    finally:
        connection.close()


def main():
    fields = parse_qs(sys.stdin.read(), keep_blank_values=True)
    show_list = (
        fields.get("method", [""])[0] == "DELETE"
        and fields.get("return_list", [""])[0] == "yes"
    )

    try:
        status, result = send_request(fields)
    except (ValueError, KeyError, OSError, http.client.HTTPException) as error:
        status = 500
        result = f"Request failed: {error}"

    if show_list:
        filename = fields.get("filename", [""])[0]
        message = f"Deleted {filename}." if 200 <= status < 300 else result
        render_file_list(message)
    else:
        print("Content-Type: text/plain; charset=utf-8")
        print()
        print(result)


if __name__ == "__main__":
    main()
