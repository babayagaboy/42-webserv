#!/usr/bin/env python3
"""Evaluator-style HTTP tests for Webserv.

Run from the repository root with: python3 tests/evaluation.py
"""

import json
import os
import socket
import subprocess
import tempfile
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PORT = 18820
SECOND_PORT = 18821


class TestFailure(Exception):
    pass


def make_config():
    return """server {
    listen %d;
    server_name localhost;
    host 127.0.0.1;
    client_max_size 100;

    location / {
        root /www/html;
        index index.html;
        allowed GET POST HEAD OPTIONS;
        cgi .py bin/session.py;
        autoindex false;
    }
    location /session.py { allowed GET; cgi .py bin/session.py; }
    location /create_file.py { allowed POST; cgi .py ./bin/create_file.py; }
    location /delete_file.py { allowed DELETE; cgi .py ./bin/delete_file.py; }
    location /patch_file.py { allowed PATCH; cgi .py ./bin/patch_file.py; }
    location /files { root /files; allowed GET POST DELETE PATCH; autoindex true; }
}

server {
    listen %d;
    server_name localhost;
    host 127.0.0.1;
    client_max_size 100;
    location / { allowed GET; }
}
""" % (PORT, SECOND_PORT)


def http_request(port, method, path, body=b"", headers=None, timeout=3):
    headers = dict(headers or {})
    headers.setdefault("Host", "localhost")
    headers.setdefault("Connection", "close")
    if body:
        headers.setdefault("Content-Length", str(len(body)))
    request = (method + " " + path + " HTTP/1.1\r\n").encode("ascii")
    request += b"".join(
        (name + ": " + str(value) + "\r\n").encode("ascii")
        for name, value in headers.items()
    )
    request += b"\r\n" + body

    sock = socket.create_connection(("127.0.0.1", port), timeout)
    sock.settimeout(timeout)
    sock.sendall(request)
    response = b""
    try:
        while True:
            chunk = sock.recv(65536)
            if not chunk:
                break
            response += chunk
    except socket.timeout:
        pass
    finally:
        sock.close()

    if not response:
        raise TestFailure("no response for %s %s" % (method, path))
    header_end = response.find(b"\r\n\r\n")
    if header_end == -1:
        raise TestFailure("invalid response headers for %s %s" % (method, path))
    header_lines = response[:header_end].decode("iso-8859-1").split("\r\n")
    headers_out = {}
    for line in header_lines[1:]:
        if ":" in line:
            name, value = line.split(":", 1)
            headers_out[name.lower()] = value.strip()
    return response.split(b"\r\n", 1)[0].decode("ascii", "replace"), headers_out, response[header_end + 4:]


def expect(label, actual, expected):
    if actual != expected:
        raise TestFailure("%s: expected %s, got %s" % (label, expected, actual))
    print("ok: %s" % label)


def expect_body(label, body, text):
    if text.encode() not in body:
        raise TestFailure("%s: response did not contain %r" % (label, text))
    print("ok: %s" % label)


def wait_for_server(process):
    deadline = time.time() + 5
    while time.time() < deadline:
        if process.poll() is not None:
            raise TestFailure("server exited with code %s" % process.returncode)
        try:
            http_request(PORT, "GET", "/")
            return
        except (OSError, TestFailure):
            time.sleep(0.05)
    raise TestFailure("server did not start")


def run():
    subprocess.check_call(["make"], cwd=ROOT)
    config = tempfile.NamedTemporaryFile(
        mode="w", suffix=".conf", prefix="webserv-evaluation-", delete=False
    )
    config.write(make_config())
    config.close()
    process = subprocess.Popen(
        [os.path.join(ROOT, "webserv"), config.name],
        cwd=ROOT,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    temporary_files = []
    try:
        wait_for_server(process)

        status, headers, body = http_request(PORT, "GET", "/")
        expect("GET static page", status, "HTTP/1.1 200 OK")
        expect("GET has content length", "content-length" in headers, True)

        status, headers, body = http_request(PORT, "HEAD", "/")
        expect("HEAD", status, "HTTP/1.1 200 OK")
        expect("HEAD has no body", body, b"")

        status, headers, body = http_request(PORT, "OPTIONS", "/")
        expect("OPTIONS", status, "HTTP/1.1 204 No Content")
        expect("OPTIONS advertises GET", "GET" in headers.get("allow", ""), True)

        status, headers, body = http_request(PORT, "GET", "/session.py")
        expect("GET CGI", status, "HTTP/1.1 200 OK")
        expect_body("GET CGI body", body, "Webserv Session Test")

        status, headers, body = http_request(PORT, "TRACE", "/")
        expect("unsupported method", status, "HTTP/1.1 405 Method Not Allowed")

        status, headers, body = http_request(PORT, "BAD", "")
        expect("malformed request", status, "HTTP/1.1 400 Bad Request")

        status, headers, body = http_request(
            PORT, "POST", "/create_file.py", b"x" * 101,
            {"Content-Type": "text/plain"}
        )
        expect("body size limit", status, "HTTP/1.1 413 Payload Too Large")

        name = ".webserv-evaluation-file"
        path = os.path.join(ROOT, "files", name)
        temporary_files.append(path)
        content = json.dumps({"filename": name, "content": "uploaded"}).encode()
        status, headers, body = http_request(
            PORT, "POST", "/create_file.py", content,
            {"Content-Type": "application/json"}
        )
        expect("POST upload CGI", status, "HTTP/1.1 201 Created")

        status, headers, body = http_request(PORT, "GET", "/files/" + name)
        expect("retrieve uploaded file", status, "HTTP/1.1 200 OK")
        expect_body("uploaded content", body, "uploaded")

        status, headers, body = http_request(PORT, "DELETE", "/files/" + name)
        expect("DELETE file", status, "HTTP/1.1 204 No Content")
        if os.path.exists(path):
            raise TestFailure("DELETE did not remove uploaded file")
        print("ok: DELETE removed uploaded file")

        patch_name = ".webserv-evaluation.json"
        patch_path = os.path.join(ROOT, "files", patch_name)
        temporary_files.append(patch_path)
        with open(patch_path, "w") as patch_file:
            json.dump({"value": 1}, patch_file)
        patch = json.dumps({
            "filename": patch_name,
            "changes": {"value": 2}
        }).encode()
        status, headers, body = http_request(
            PORT, "PATCH", "/patch_file.py", patch,
            {"Content-Type": "application/json"}
        )
        expect("PATCH CGI", status, "HTTP/1.1 200 OK")
        with open(patch_path) as patch_file:
            if json.load(patch_file) != {"value": 2}:
                raise TestFailure("PATCH wrote unexpected JSON")
        print("ok: PATCH updated JSON")

        status, headers, body = http_request(SECOND_PORT, "GET", "/")
        expect("second configured port", status, "HTTP/1.1 404 Not Found")

        print("All evaluation tests passed.")
    finally:
        process.terminate()
        try:
            process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()
        for path in temporary_files:
            if os.path.exists(path):
                os.unlink(path)
        os.unlink(config.name)


if __name__ == "__main__":
    try:
        run()
    except (OSError, subprocess.CalledProcessError, TestFailure) as error:
        print("TEST FAILURE: %s" % error)
        raise SystemExit(1)
