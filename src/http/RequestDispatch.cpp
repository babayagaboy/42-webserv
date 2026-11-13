#include <RequestHandlers.hpp>
#include <HTTPresponse.hpp>
#include <Server.hpp>

static bool handleConfiguredReturn(Client &client, Server &server,
	const Location &location)
{
	const std::vector<std::pair<int, std::string> > &returns = location.getReturn();
	if (returns.empty())
		return false;

	const std::pair<int, std::string> &directive = returns.front();
	HTTPresponse response;
	std::vector<std::pair<std::string, std::string> > headers;
	if (!directive.second.empty())
		headers.push_back(std::make_pair("Location", directive.second));
	headers.push_back(std::make_pair("Content-Length", "0"));
	response.setStatusCode(directive.first);
	response.setHeaders(headers);

	client.sendBuffer = response.buildResponse();
	client.sendOffset = 0;
	server.enableClientWrite(client.fd);
	return true;
}

void processRequest(Client &client, Server &server)
{
	int location = server.findLocation(client);
	if (location < 0)
	{
		server.handleError(client, -1, 404);
		return;
	}

	if (handleConfiguredReturn(client, server, server.serversConfs.getLocations()[location]))
		return;

	if (!server.isMethodAllowed(client.request.method, location))
	{
		server.handleError(client, location, 405);
		return;
	}

	if (client.request.method == "POST" || client.request.method == "PUT"
		|| client.request.method == "PATCH" || client.request.method == "DELETE")
		server.handleSession(client);

	std::string methods[] = {
		"GET", "POST", "DELETE", "PUT", "HEAD", "OPTIONS", "PATCH"
	};
	int (*methodFunctions[])(Client &, Server &, int) = {
		&method_GET, &method_POST, &method_DELETE, &method_PUT,
		&method_HEAD, &method_OPTIONS, &method_PATCH
	};
	for (size_t i = 0; i < 7; ++i)
	{
		if (client.request.method == methods[i])
		{
			methodFunctions[i](client, server, location);
			return;
		}
	}
	server.handleError(client, location, 405);
}