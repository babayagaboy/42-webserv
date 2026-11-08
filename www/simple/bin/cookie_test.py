"""Set, read, or expire only the cookie used by the Inspection page."""

import json
import os
import sys
from http.cookies import CookieError, SimpleCookie
from urllib.parse import parse_qs


def main():
    method = os.environ.get("REQUEST_METHOD", "GET")
    form = parse_qs(sys.stdin.read()) if method == "POST" else {}
    action = form.get("action", ["check"])[0]
    status = "200 OK"
    set_cookie = ""
    result = {}

    if action not in ("set", "check", "clear"):
        status = "400 Bad Request"
        result["error"] = "Choose Set, Check, or Clear cookie."
    elif method != ("GET" if action == "check" else "POST"):
        status = "405 Method Not Allowed"
        result["error"] = "Use GET to check and POST to set or clear the cookie."
        print("Allow: " + ("GET" if action == "check" else "POST"))
    else:
        cookies = SimpleCookie()
        try:
            cookies.load(os.environ.get("HTTP_COOKIE", ""))
        except CookieError:
            cookies.clear()

        cookie = cookies.get("webserv_test")
        result["received"] = cookie.value if cookie is not None else None

        if action == "set":
            set_cookie = "webserv_test=hello; Path=/; Max-Age=86400; HttpOnly; SameSite=Lax"
        elif action == "clear":
            set_cookie = "webserv_test=; Path=/; Max-Age=0; HttpOnly; SameSite=Lax"

        if set_cookie:
            print("Set-Cookie: " + set_cookie)
            result["set_cookie"] = set_cookie

    print("Status: " + status)
    print("Content-Type: application/json; charset=utf-8")
    print("Cache-Control: no-store")
    print()
    print(json.dumps(result))


if __name__ == "__main__":
    main()
