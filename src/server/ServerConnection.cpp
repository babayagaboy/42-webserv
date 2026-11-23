#include <Server.hpp>
#include <HTTPMessageParser.hpp>
#include <RequestHandlers.hpp>
#include <cerrno>
#include <cctype>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <sys/socket.h>

int parseContentLength(const std::string &value, size_t &length);
int decodeChunkedBody(const std::string &buffer, size_t bodyStart,
						 std::string &body, size_t &consumed,
						 size_t maxBodySize);

void Server::acceptNewClient()
{
	int clientFd = accept(serverSocket, (struct sockaddr *)&socketAddress, &address_size);

	if (clientFd == -1)
	{
		std::cerr	<< "accept() failed: "
					<< serverSocket
					<< ":" 
					<< strerror(errno) << std::endl;
		return;
	}

	clients[clientFd] = Client();
	clients[clientFd].fd = clientFd;
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
	{
		close(clientFd);
		return;
	}

	pollfd clientPollFd = {};
	clientPollFd.fd = clientFd;
	clientPollFd.events = POLLIN;

	pollfds_vector.push_back(clientPollFd);
}

bool Server::receiveFromClient(size_t i)
{
    int clientFd = pollfds_vector[i].fd;
    Client &client = clients[clientFd];

    char buff[4096];

    client.bytes_read = recv(client.fd, buff, sizeof(buff), 0);

	if (client.bytes_read < 0)
	{
		disconnectClient(i);
		return true;
	}

	if (client.bytes_read == 0)
		return (disconnectClient(i), true);

    client.recvBuffer.append(buff, client.bytes_read);

	size_t headerEnd = client.recvBuffer.find("\r\n\r\n");

	if (headerEnd == std::string::npos && client.recvBuffer.size() > 8192)
	{
		client.recvBuffer.clear();
		handleError(client, -1, 400);
		return false;
	}

	if (headerEnd == std::string::npos)
        return false;

	if (client.request.method.empty())
	{
		std::stringstream ss(client.recvBuffer);
		client.request = fill_HTTP_object(ss);
	}
	if (client.request.method.empty() || client.request.path.empty()
		|| client.request.version.empty())
	{
		client.recvBuffer.clear();
		handleError(client, -1, 400);
		return false;
	}
	size_t bodyMaxSize = this->serversConfs.getClientMaxSize();
	int requestLocation = findLocation(client);
	if (requestLocation >= 0
		&& serversConfs.getLocations()[requestLocation].getClientMaxSize() != 0)
		bodyMaxSize = serversConfs.getLocations()[requestLocation].getClientMaxSize();

	size_t contentLength = 0;
	bool chunked = false;

    std::map<std::string, std::string>::const_iterator it =
        client.request.headers.find("Content-Length");

    if (it != client.request.headers.end())
    {
		if (!parseContentLength(it->second, contentLength))
		{
			client.recvBuffer.clear();
			handleError(client, -1, 400);
			return false;
		}
    }

	it = client.request.headers.find("Transfer-Encoding");
	if (it != client.request.headers.end())
	{
		std::string encoding = it->second;
		for (size_t n = 0; n < encoding.size(); ++n)
			encoding[n] = static_cast<char>(std::tolower(static_cast<unsigned char>(encoding[n])));
		if (encoding != "chunked")
		{
			client.recvBuffer.clear();
			handleError(client, -1, 400);
			return false;
		}
		if (client.request.headers.find("Content-Length") != client.request.headers.end())
		{
			client.recvBuffer.clear();
			handleError(client, -1, 400);
			return false;
		}
		chunked = true;
	}

    size_t bodyStart = headerEnd + 4;
	std::string decodedBody;
	size_t requestSize = 0;

	if (chunked)
    {
		if (bodyMaxSize != 0)
			client.recvBuffer.reserve(bodyStart + bodyMaxSize + 65536);
		if (bodyMaxSize != 0 && bodyMaxSize <= 1024 * 1024
			&& client.recvBuffer.size() - bodyStart > bodyMaxSize + 65536)
		{
			client.recvBuffer.clear();
			client.closeAfterResponse = true;
			handleError(client, -1, 413);
			return false;
		}
		bool hasChunkTerminator =
			client.recvBuffer.compare(bodyStart, 3, "0\r\n") == 0
			|| (client.recvBuffer.size() >= 5
				&& client.recvBuffer.compare(client.recvBuffer.size() - 5,
					5, "0\r\n\r\n") == 0);
		if (!hasChunkTerminator)
			return false;
		int result = decodeChunkedBody(
			client.recvBuffer, bodyStart, decodedBody, requestSize,
			bodyMaxSize);
		if (result == 0)
			return false;
		if (result == -2)
		{
			client.recvBuffer.clear();
			client.closeAfterResponse = true;
			handleError(client, -1, 413);
			return false;
		}
		if (result < 0)
		{
			client.recvBuffer.clear();
			handleError(client, -1, 400);
			return false;
		}
		client.request.body.swap(decodedBody);
    }
	else
	{
		if (bodyMaxSize != 0 && contentLength > bodyMaxSize)
		{
			client.recvBuffer.clear();
			client.closeAfterResponse = true;
			handleError(client, -1, 413);
			return false;
		}
		if (client.recvBuffer.size() - bodyStart < contentLength)
			return false;
		client.request.body.assign(client.recvBuffer, bodyStart, contentLength);
		requestSize = bodyStart + contentLength;
	}

	if (bodyMaxSize != 0 && client.request.body.size() > bodyMaxSize)
	{
		client.recvBuffer.clear();
		client.closeAfterResponse = true;
		handleError(client, -1, 413);
		return false;
	}

    processRequest(client, *this);
	client.request = HTTPrequest();

	if (requestSize == client.recvBuffer.size())
		std::string().swap(client.recvBuffer);
	else
		client.recvBuffer.erase(0, requestSize);

    return false;
}

void Server::disconnectClient(size_t i)
{
    int clientFd = pollfds_vector[i].fd;

    close(clientFd);
    clients.erase(clientFd);
    pollfds_vector.erase(pollfds_vector.begin() + i);
}
