*This project has been created as part of the 42 curriculum by hgutterr, myivanov.*

# Description

Webserv is a C++98 HTTP server implementing a poll-driven event loop. It serves
static files, handles configured routes and methods, accepts request bodies,
supports uploads and CGI execution, and can listen on multiple ports.

# Instructions

Build the project with:

	make

Run it with a configuration file:

	./webserv simple.conf

The configuration files in the repository contain examples for static files,
method restrictions, redirects, directory listing, CGI scripts, uploads, and
multiple listening ports. Clean build artifacts with `make clean` and remove
the executable with `make fclean`.

# Project Layout

- `src/app/` starts the server and opens listening sockets.
- `src/config/` contains configuration models and directive validation.
- `src/http/` parses HTTP messages, dispatches requests, implements method
	handlers, and converts CGI output into HTTP responses.
- `src/server/` owns client connections, polling, sessions, and runtime setup,
	including loading the configuration into server objects.
- `inc/` contains the shared interfaces grouped by the same subsystem.
- `tests/` contains the evaluation and regression scripts.

Request dispatch is separate from method handling: `RequestDispatch.cpp`
selects the route and HTTP method, while `RequestHandlers.cpp` implements the
method behavior. Interfaces for these modules are declared in `inc/http/` and
`inc/config/` rather than repeated as local forward declarations.

# Resources

- RFC 7230, HTTP/1.1 message syntax and routing
- RFC 7231, HTTP/1.1 semantics and methods
- RFC 3875, CGI 1.1
- `man 2 poll`, `man 2 socket`, `man 2 accept`, and `man 2 fork`
- NGINX documentation and the included project tester

AI was used to help audit the implementation against the subject, identify
nonblocking I/O and HTTP framing risks, suggest focused tests, and review
configuration and CGI behavior. All changes were compiled, tested locally,
and reviewed against the project requirements.