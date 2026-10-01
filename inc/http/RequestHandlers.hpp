#ifndef REQUEST_HANDLERS_HPP
# define REQUEST_HANDLERS_HPP

class Client;
class Server;

int method_GET(Client &client, Server &server, int location);
int method_POST(Client &client, Server &server, int location);
int method_DELETE(Client &client, Server &server, int location);
int method_PUT(Client &client, Server &server, int location);
int method_HEAD(Client &client, Server &server, int location);
int method_OPTIONS(Client &client, Server &server, int location);
int method_PATCH(Client &client, Server &server, int location);

void processRequest(Client &client, Server &server);

#endif