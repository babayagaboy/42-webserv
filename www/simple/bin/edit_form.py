#!/usr/bin/env python3

"""Fill the edit form's file selection each time the page is opened."""

from html import escape
from pathlib import Path


def main():
    website = Path(__file__).resolve().parent.parent

    try:
        filenames = sorted(
            path.name for path in (website / "files").iterdir() if path.is_file()
        )
        page = (website / "update.html").read_text(encoding="utf-8")
    except OSError:
        print("Status: 500 Internal Server Error")
        print("Content-Type: text/plain; charset=utf-8")
        print()
        print("Could not load the saved files. Please try again.")
        return

    options = []

    for filename in filenames:
        name = escape(filename, quote=True)
        options.append(f'<option value="{name}">{name}</option>')

    if not options:
        options.append('<option value="" disabled>No saved files yet</option>')

    page = page.replace("<!-- SAVED_FILE_OPTIONS -->", "\n                    ".join(options))
    print("Content-Type: text/html; charset=utf-8")
    print("Cache-Control: no-store")
    print()
    print(page)


if __name__ == "__main__":
    main()
