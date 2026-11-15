#include <Server.hpp>
#include "HTTPresponse.hpp"
#include <fcntl.h>
#include <sstream>
#include <utility>
#include <unistd.h>

bool Server::sendToClient(size_t i)
{
    int fd = pollfds_vector[i].fd;
    Client &c = clients[fd];

    if (c.sendOffset >= c.sendBuffer.size()) {
        pollfds_vector[i].events &= ~POLLOUT;
		if (c.closeAfterResponse)
		{
			disconnectClient(i);
			return true;
		}
        return false;
    }

    ssize_t sent = send(fd, c.sendBuffer.c_str() + c.sendOffset, c.sendBuffer.size() - c.sendOffset, 0 );

	if (sent <= 0)
	{
		disconnectClient(i);
		return true;
	}

    c.sendOffset += sent;

    if (c.sendOffset >= c.sendBuffer.size()) {
        c.sendBuffer.clear();
        c.sendOffset = 0;

        pollfds_vector[i].events &= ~POLLOUT;
		if (c.closeAfterResponse)
		{
			disconnectClient(i);
			return true;
		}
    }

    return false;
}

void Server::enableClientWrite(int fd)
{
    for (size_t i = 0; i < pollfds_vector.size(); ++i)
    {
        if (pollfds_vector[i].fd == fd)
        {
            pollfds_vector[i].events |= POLLOUT;
            return;
        }
    }
}

int Server::handleError(Client &c, int l, int statusCode)
{
	HTTPresponse response;
	std::string body;
	std::string errorPage;

	if (l >= 0)
	{
		const Location &location =
			serversConfs.getLocations()[l];

		const std::vector<std::pair<int, std::string> > &errors =
			location.getErrorPage();

		for (size_t i = 0; i < errors.size(); ++i)
		{
			if (errors[i].first == statusCode)
			{
				errorPage = errors[i].second;
				break;
			}
		}

		if (!errorPage.empty())
		{
			std::string root = location.getDefaultRoot();

			if (!root.empty() && root[0] == '/')
				root = "." + root;

			if (!root.empty()
				&& root[root.size() - 1] != '/')
			{
				root += '/';
			}

			while (!errorPage.empty()
				&& errorPage[0] == '/')
			{
				errorPage.erase(0, 1);
			}

			std::string path = root + errorPage;

			int fd = open(path.c_str(), O_RDONLY);

			if (fd >= 0)
			{
				char buffer[4096];
				ssize_t bytesRead;

				while ((bytesRead =
					read(fd, buffer, sizeof(buffer))) > 0)
				{
					body.append(buffer, bytesRead);
				}

				close(fd);
				if (bytesRead < 0)
					body.clear();
			}
		}
	}

	if (body.empty())
	{
		std::stringstream ss;

		ss << statusCode;

		body =
			"<!DOCTYPE html>\n"
			"<html>\n"
			"<head>\n"
			"<title>Error "
			+ ss.str()
			+ "</title>\n"
			"</head>\n"
			"<body>\n"
			"<h1>Error "
			+ ss.str()
			+ "</h1>\n"
			"<p>The server encountered an error.</p>\n"
			"</body>\n"
			"</html>\n";
	}

	std::stringstream ss;
	ss << body.size();

	std::vector<std::pair<std::string, std::string> > headers;

	headers.push_back(
		std::make_pair("Content-Length", ss.str())
	);

	headers.push_back(
		std::make_pair("Content-Type", "text/html")
	);

	response.setStatusCode(statusCode);
	response.setHeaders(headers);
	response.setBody(body);

	c.sendBuffer = response.buildResponse();
	c.sendOffset = 0;

	enableClientWrite(c.fd);

	return 1;
}
