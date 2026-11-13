"""Read or save the website's color theme using a server cookie."""

import json
import os
import sys
from http.cookies import CookieError, SimpleCookie
from urllib.parse import parse_qs


def main():
    cookies = SimpleCookie()
    try:
        cookies.load(os.environ.get("HTTP_COOKIE", ""))
    except CookieError:
        cookies.clear()

    cookie = cookies.get("webserv_theme")
    theme = cookie.value if cookie and cookie.value in ("light", "dark") else "light"
    method = os.environ.get("REQUEST_METHOD", "GET")
    status = "200 OK"
    result = {"theme": theme}

    if method == "POST":
        form = parse_qs(sys.stdin.read())
        theme = form.get("theme", [""])[0]
        if theme not in ("light", "dark"):
            status = "400 Bad Request"
            result = {"error": "Choose light or dark mode."}
        else:
            # JavaScript reads this non-sensitive preference before the page paints.
            print(f"Set-Cookie: webserv_theme={theme}; Path=/; Max-Age=31536000; SameSite=Lax")
            result = {"theme": theme}
    elif method != "GET":
        status = "405 Method Not Allowed"
        print("Allow: GET, POST")
        result = {"error": "Use GET to read or POST to save the theme."}

    print("Status: " + status)
    print("Content-Type: application/json; charset=utf-8")
    print("Cache-Control: no-store")
    print()
    print(json.dumps(result))


if __name__ == "__main__":
    main()
