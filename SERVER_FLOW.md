# Webserv execution flow

Use this as a presentation walkthrough: configuration is loaded first, then the
server polls sockets, dispatches requests, and writes responses back.

## Flow at a glance

```text
main
  -> fillServerConfig
  -> create, bind, and listen on configured server sockets
  -> Server::run
       -> accept a new connection
       -> receive and parse a request
       -> processRequest
            -> select location and validate method
            -> call the matching HTTP method handler
            -> optionally start a CGI process
       -> build a response and enable POLLOUT
       -> send response to the client
```

CGI uses the same polling loop: its input and output pipes are added to the
poll list, written/read as they become ready, and its output is converted into
an HTTP response.

## Walkthrough

1. **Load configuration.** `main` requires a configuration-file argument and
   calls `fillServerConfig`. The loader tokenizes and validates the file, then
   fills each server's `ServerConf` and locations.
2. **Start listeners.** `main` creates a nonblocking listening socket for each
   configured server, binds and listens, stores each listener in its poll list,
   links the servers as peers, and starts `Server::run` on the first server.
3. **Wait for work.** `Server::run` combines the poll lists of peer servers,
   calls `poll`, and dispatches each ready descriptor. It distinguishes
   listening sockets, CGI pipes, and client sockets.
4. **Accept and read.** A ready listener calls `acceptNewClient`. For a client
   socket, `receiveFromClient` accumulates bytes, parses the request headers,
   waits until the declared or chunked body is complete, then calls
   `processRequest`.
5. **Route and handle.** `processRequest` selects the best matching location,
   checks configured redirects and allowed methods, creates/updates sessions
   for write methods, and dispatches to the handler for the HTTP method.
6. **Create a response.** A handler either fills the client's send buffer or
   starts CGI. Error paths use `handleError`. CGI output is collected and
   converted by `sendCGIResponse`.
7. **Write to the client.** Response creation enables `POLLOUT`. On the next
   poll event, `sendToClient` sends the buffered response and clears or closes
   the connection when appropriate.

## Function and file map

| Responsibility | Important functions and locations |
|---|---|
| Startup and listener setup | [`main`](./src/app/main.cpp#L70), `create_server_socket` and `configureSocketAddress` in [`main.cpp`](./src/app/main.cpp#L27) |
| Configuration loading | [`fillServerConfig`](./src/config/configFile.cpp#L381), `tokenizeConfigFile` at [`configFile.cpp:26`](./src/config/configFile.cpp#L26), `parseConfigFile` at [`configFile.cpp:314`](./src/config/configFile.cpp#L314) |
| Poll loop and dispatch | [`Server::run`](./src/server/Server.cpp#L42) |
| Client connections and request input | [`Server::acceptNewClient`](./src/server/ServerConnection.cpp#L17), [`Server::receiveFromClient`](./src/server/ServerConnection.cpp#L45), [`Server::disconnectClient`](./src/server/ServerConnection.cpp#L207) |
| Request parsing | [`fill_HTTP_object`](./src/http/fillHTTPobject.cpp#L97), `parseContentLength` and `decodeChunkedBody` in [`RequestBodyParsing.cpp`](./src/server/RequestBodyParsing.cpp#L17) |
| Routing and method dispatch | [`processRequest`](./src/http/RequestDispatch.cpp#L27), [`Server::findLocation`](./src/server/ServerRouting.cpp#L3), [`Server::isMethodAllowed`](./src/server/ServerRouting.cpp#L37) |
| HTTP method handlers | `method_GET` at [`RequestHandlers.cpp:586`](./src/http/RequestHandlers.cpp#L586); `method_POST`, `method_DELETE`, `method_PUT`, `method_OPTIONS`, `method_PATCH`, and `method_HEAD` are in the same file |
| Session state | `Server::handleSession` and related session methods in [`ServerSession.cpp`](./src/server/ServerSession.cpp#L5) |
| CGI setup and environment | HTTP method handlers in [`RequestHandlers.cpp`](./src/http/RequestHandlers.cpp#L29); `buildEnvironment` and `findCGIcompiler` in [`RequestSupport.cpp`](./src/http/RequestSupport.cpp#L55) |
| CGI pipe I/O and cleanup | `Server::sendToCgi`, `Server::receiveFromCgi`, and `Server::cleanupExpiredCgi` in [`ServerCgi.cpp`](./src/server/ServerCgi.cpp#L11) |
| Response creation and delivery | [`Server::handleError`](./src/server/ServerResponse.cpp#L60), [`Server::sendToClient`](./src/server/ServerResponse.cpp#L8), and [`HTTPresponse::buildResponse`](./src/http/HTTPresponse.cpp#L82) |

## A few concepts to explain

- **`Server` owns the event loop.** It tracks the server/client/CGI file
  descriptors and routes ready events to the relevant method. The declarations
  and shared state are in [`Server.hpp`](./inc/server/Server.hpp).
- **`Client` keeps per-connection state.** Its request, receive buffer, send
  buffer, session ID, and CGI descriptors let the event loop resume work across
  multiple poll iterations. See [`Client.hpp`](./inc/server/Client.hpp).
- **Responses are buffered.** Handlers construct a response and store it in
  `Client::sendBuffer`; the event loop later sends it when the client socket is
  writable. This handles partial writes without blocking the whole server.
- **CGI is asynchronous relative to the event loop.** Method handlers create
  pipes and register them with polling; the loop writes the request body and
  collects CGI output before queuing the HTTP response.
