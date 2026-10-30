#!/usr/bin/env python3

"""Render saved files as ordinary HTML links, without browser JavaScript."""

from html import escape
from pathlib import Path
from urllib.parse import quote


def render_file_list(message=""):
    files_directory = Path(__file__).resolve().parent.parent / "files"

    try:
        files = sorted(path.name for path in files_directory.iterdir() if path.is_file())
    except OSError:
        print("Status: 500 Internal Server Error")
        print("Content-Type: text/plain")
        print()
        print("Could not read the files directory")
        raise SystemExit

    print("Content-Type: text/html; charset=utf-8")
    print()
    print("""<!doctype html>
    <html lang="en">
    <head>
        <link rel="icon" type="image/png" href="/images/web-server.png">
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>Saved files - Webserver</title>
        <link rel="stylesheet" href="/styles/site.css">
    </head>
    <body>
        <main class="embedded">
            <h1>Saved files</h1>
            <p>Open, download, or permanently delete a file.</p>
    """)

    if message:
        print(f'        <p role="status">{escape(message)}</p>')

    if not files:
        print("        <p>No files have been uploaded yet.</p>")
    else:
        print('        <ul class="file-list">')

        for filename in files:
            name = escape(filename, quote=True)
            url = "/files/" + quote(filename, safe="")

            print(f"""            <li>
                    <a href="{url}" target="_blank" rel="noopener">{name}</a>
                    <a href="{url}" download="{name}">Download</a>
                    <form action="/method_test.py" method="post" class="delete-form">
                        <input type="hidden" name="method" value="DELETE">
                        <input type="hidden" name="filename" value="{name}">
                        <input type="hidden" name="return_list" value="yes">
                        <button type="submit" aria-label="DELETE {name}">DELETE</button>
                    </form>
                </li>""")

        print("        </ul>")

    print("""    </main>
    </body>
    </html>
    """)


if __name__ == "__main__":
    render_file_list()
