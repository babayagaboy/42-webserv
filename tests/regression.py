#!/usr/bin/env python3
import os
import socket
import subprocess
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def request(raw, port=8808):
    sock = socket.create_connection(("127.0.0.1", port), 3)
    sock.sendall(raw)
    sock.settimeout(3)
    data = b""
    try:
        while True:
            part = sock.recv(4096)
            if not part:
                break
            data += part
    except socket.timeout:
        pass
    sock.close()
    return data.split(b"\r\n", 1)[0].decode("ascii", "replace")


def check(actual, expected, label):
    if actual != expected:
        raise AssertionError("%s: expected %s, got %s" % (label, expected, actual))
    print("ok: %s" % label)


subprocess.check_call(["make"], cwd=ROOT)
server = subprocess.Popen(["./webserv", "simple.conf"], cwd=ROOT)
try:
    deadline = time.time() + 3
    while time.time() < deadline:
        try:
            request(b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")
            break
        except OSError:
            time.sleep(0.05)
    else:
        raise RuntimeError("server did not start")

    check(request(b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"),
          "HTTP/1.1 200 OK", "GET")
    check(request(b"HEAD / HTTP/1.1\r\nHost: localhost\r\n\r\n"),
          "HTTP/1.1 200 OK", "HEAD")
    check(request(b"TRACE / HTTP/1.1\r\nHost: localhost\r\n\r\n"),
          "HTTP/1.1 405 Method Not Allowed", "unsupported TRACE")
    check(request(b"POST /create_file.py HTTP/1.1\r\nHost: localhost\r\n"
                   b"Content-Length: nope\r\n\r\n"),
          "HTTP/1.1 400 Bad Request", "invalid Content-Length")
    check(request(b"POST /create_file.py HTTP/1.1\r\nHost: localhost\r\n"
                   b"Content-Length: 100000001\r\n\r\n"),
          "HTTP/1.1 413 Payload Too Large", "declared body limit")
    check(request(b"POST /create_file.py HTTP/1.1\r\nHost: localhost\r\n"
                   b"Transfer-Encoding: chunked\r\n\r\n"
                   b"3\r\nabc\r\n0\r\n\r\n"),
          "HTTP/1.1 500 Internal Server Error", "chunked CGI framing")
finally:
    server.terminate()
    try:
        server.wait(timeout=3)
    except subprocess.TimeoutExpired:
        server.kill()
        server.wait()
