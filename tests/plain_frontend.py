#!/usr/bin/env python3
"""Check static frontend delivery using an isolated website and local server.

Run from the repository root: python3 tests/plain_frontend.py
Only temporary files are created or deleted. Build webserv first with make.
"""

import http.client
import json
from pathlib import Path
import shutil
import socket
import subprocess
import tempfile
import time
from urllib.parse import urlencode

ROOT = Path(__file__).resolve().parents[1]


def free_port():
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


def request(port, path, method="GET", body=None, headers=None):
    connection = http.client.HTTPConnection("127.0.0.1", port, timeout=3)
    try:
        connection.request(method, path, body=body, headers=headers or {})
        response = connection.getresponse()
        return response.status, dict(response.getheaders()), response.read()
    finally:
        connection.close()


def run():
    port = free_port()
    with tempfile.TemporaryDirectory(prefix="webserv-frontend-") as temporary:
        folder = Path(temporary)
        website = folder / "www"
        shutil.copytree(
            ROOT / "www/simple", website,
            ignore=shutil.ignore_patterns("files", "react", "__pycache__"),
        )
        (website / "files").mkdir()
        filename = 'space "quote".txt'
        (website / "files" / filename).write_text("example", encoding="utf-8")
        (folder / "outside.txt").write_text("outside root", encoding="utf-8")
        config = folder / "test.conf"
        config.write_text(f"""server {{
    listen {port};
    host 127.0.0.1;
    server_name localhost;
    location / {{
        root /www;
        index index.html;
        allowed GET HEAD OPTIONS;
        error_page 404 errors/404.html;
    }}
    location /files {{ root /www/files; allowed GET; autoindex true; }}
    location /redirect {{ allowed GET; return 301 /manager.html; }}
    location /create_file.py {{ allowed POST; cgi .py ./www/bin/create_file.py; }}
    location /list_files.py {{ allowed GET; cgi .py ./www/bin/list_files.py; }}
    location /edit_form.py {{ allowed GET; cgi .py ./www/bin/edit_form.py; }}
    location /method_test.py {{ allowed POST; cgi .py ./www/bin/method_test.py; }}
    location /put_file.py {{ allowed PUT; cgi .py ./www/bin/put_file.py; }}
    location /patch_file.py {{ allowed PATCH; cgi .py ./www/bin/patch_file.py; }}
    location /delete_file.py {{ allowed DELETE; cgi .py ./www/bin/delete_file.py; }}
    location /upload_file.php {{ allowed POST; cgi .php ./www/bin/upload_file.php; }}
}}
""", encoding="utf-8")
        with (folder / "server.log").open("w") as log:
            server = subprocess.Popen(
                [str(ROOT / "webserv"), str(config)], cwd=folder,
                stdout=log, stderr=log,
            )
            try:
                for attempt in range(50):
                    if server.poll() is not None:
                        raise RuntimeError((folder / "server.log").read_text())
                    try:
                        request(port, "/")
                        break
                    except OSError:
                        time.sleep(0.1)
                else:
                    raise RuntimeError("Server did not become ready")

                pages = {
                    "/": "text/html",
                    "/manager.html": "text/html",
                    "/editor.html": "text/html",
                    "/edit_form.py": "text/html",
                    "/styles/site.css": "text/css",

                    "/images/server-racks.jpg": "image/jpeg",
                    "/images/web-server.png": "image/png",
                }
                for path, expected_type in pages.items():
                    status, headers, body = request(port, path)
                    assert status == 200 and body, path
                    assert headers["Content-Type"].startswith(expected_type), path
                homepage = request(port, "/")[2]
                for link in (b'/manager.html', b'/editor.html', b'/edit_form.py'):
                    assert b'href="' + link + b'"' in homepage
                assert b'onclick=' not in homepage
                assert request(port, "/styles/site.css", "HEAD")[2] == b""
                assert request(port, "/styles/site.css", "HEAD")[1]["Content-Type"].startswith("text/css")
                status, headers, body = request(port, "/missing-page")
                assert status == 404 and b"Page not found" in body
                assert request(port, "/redirect")[1]["Location"] == "/manager.html"
                assert filename in json.loads(request(port, "/files")[2])
                assert request(port, "/files/space%20%22quote%22.txt")[2] == b"example"
                assert request(port, "/files/%2e%2e/%2e%2e/outside.txt")[0] == 404
                assert request(port, "/files/bad%zz")[0] == 404
                form = urlencode({"filename": "form.txt", "content": "Hello from HTML"})
                status, _, _ = request(
                    port, "/create_file.py", "POST", form,
                    {"Content-Type": "application/x-www-form-urlencoded"},
                )
                assert status == 201
                assert request(port, "/files/form.txt")[2] == b"Hello from HTML"
                listing = request(port, "/list_files.py")[2]
                assert b"form.txt" in listing and b"&quot;" in listing
                assert b"<script" not in listing
                edit_page = request(port, "/edit_form.py")[2]
                assert b'<option value="form.txt">form.txt</option>' in edit_page
                assert b'space &quot;quote&quot;.txt' in edit_page
                assert b"SAVED_FILE_OPTIONS" not in edit_page

                if shutil.which("php-cgi"):
                    boundary = "WebservFormTest"
                    upload = (
                        f"--{boundary}\r\n"
                        'Content-Disposition: form-data; name="file"; filename="upload.txt"\r\n'
                        "Content-Type: text/plain\r\n\r\n"
                        f"--{boundary}--\r\n"
                    )
                    status, _, body = request(
                        port, "/upload_file.php", "POST", upload,
                        {"Content-Type": f"multipart/form-data; boundary={boundary}"},
                    )
                    assert status == 201, body
                else:
                    print("SKIP: multipart upload requires php-cgi")

                for method, filename, content, expected in [
                    ("HEAD", "", "", b"Status: 200"),
                    ("OPTIONS", "", "", b"Status: 204"),
                    ("PUT", "methods.txt", "first", b"Status: 200"),
                    ("PATCH", "methods.txt", "second", b"Status: 200"),
                    ("DELETE", "methods.txt", "", b"Status: 200"),
                ]:
                    form = urlencode({"method": method, "filename": filename, "content": content})
                    status, _, body = request(
                        port, "/method_test.py", "POST", form,
                        {"Content-Type": "application/x-www-form-urlencoded"},
                    )
                    assert status == 200 and expected in body, (method, body)
                assert not (website / "files/methods.txt").exists()
                quoted_filename = 'space "quote".txt'
                form = urlencode({
                    "method": "DELETE",
                    "filename": quoted_filename,
                    "return_list": "yes",
                })
                status, headers, body = request(
                    port, "/method_test.py", "POST", form,
                    {"Content-Type": "application/x-www-form-urlencoded"},
                )
                assert status == 200 and headers["Content-Type"].startswith("text/html")
                assert b'Deleted space &quot;quote&quot;.txt.' in body
                assert not (website / "files" / quoted_filename).exists()
                assert b'value="form.txt"' in body
                assert b'class="delete-form"' in body
                assert b'value="space &quot;quote&quot;.txt"' not in body

                # A repeated deletion keeps the list visible and reports the failure.
                _, _, body = request(
                    port, "/method_test.py", "POST", form,
                    {"Content-Type": "application/x-www-form-urlencoded"},
                )
                assert b"Status: 404" in body and b'value="form.txt"' in body
                assert b"Deleted space" not in body
                print("PASS: native forms, uploads, listing, per-file deletion and HTTP methods")
            finally:
                server.terminate()
                try:
                    server.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    server.kill()
                    server.wait()


if __name__ == "__main__":
    run()
