#include <Server.hpp>
#include <RequestSupport.hpp>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

void Server::cleanupExpiredCgi()
{
	const time_t timeout = 10;
	time_t now = std::time(NULL);

	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		Client &client = it->second;
		if (client.cgiPid <= 0 || client.cgiStart == 0
			|| now - client.cgiStart < timeout)
			continue;

		kill(client.cgiPid, SIGKILL);
		waitpid(client.cgiPid, NULL, 0);
		int descriptors[2] = { client.cgiInputFd, client.cgiOutputFd };
		for (int d = 0; d < 2; ++d)
		{
			if (descriptors[d] < 0)
				continue;
			close(descriptors[d]);
			for (size_t p = 0; p < pollfds_vector.size(); ++p)
			{
				if (pollfds_vector[p].fd == descriptors[d])
				{
					pollfds_vector.erase(pollfds_vector.begin() + p);
					break;
				}
			}
		}
		client.cgiInputFd = -1;
		client.cgiOutputFd = -1;
		client.cgiPid = -1;
		client.cgiStart = 0;
		handleError(client, findLocation(client), 504);
	}
}

bool	Server::isCgiOutputFd(int fd) const
{
	for (std::map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second.cgiOutputFd == fd)
			return true;
	}
	return false;
}

bool	Server::isCgiInputFd(int fd) const
{
	for (std::map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second.cgiInputFd == fd)
			return true;
	}
	return false;
}

Client *Server::findClientByCgiFd(int fd)
{
    std::map<int, Client>::iterator it;

    for (it = clients.begin(); it != clients.end(); ++it) {
        if (it->second.cgiInputFd == fd || it->second.cgiOutputFd == fd)
            return &it->second;
    }

    return NULL;
}

bool Server::sendToCgi(size_t i)
{
	int fd = pollfds_vector[i].fd;

	Client *client = findClientByCgiFd(fd);

	if (!client)
		return false;

	size_t remaining =
		client->cgiBody.size() - client->cgiBodyOffset;

	if (remaining == 0)
	{
		close(fd);
		client->cgiInputFd = -1;

		pollfds_vector.erase(
			pollfds_vector.begin() + i
		);
		return true;
	}

	ssize_t bytesWritten = write(
		fd,
		client->cgiBody.c_str() + client->cgiBodyOffset,
		remaining
	);

	if (bytesWritten == -1)
	{
		close(fd);
		client->cgiInputFd = -1;

		pollfds_vector.erase(
			pollfds_vector.begin() + i
		);

		handleError(
			*client,
			findLocation(*client),
			500
		);
		return true;
	}

	if (bytesWritten == 0)
	{
		std::cerr << "write to CGI wrote 0 bytes"
				  << std::endl;

		close(fd);
		client->cgiInputFd = -1;

		pollfds_vector.erase(
			pollfds_vector.begin() + i
		);

		handleError(
			*client,
			findLocation(*client),
			500
		);
		return true;
	}

	client->cgiBodyOffset += bytesWritten;

	if (client->cgiBodyOffset == client->cgiBody.size())
	{
		close(fd);
		client->cgiInputFd = -1;

		pollfds_vector.erase(
			pollfds_vector.begin() + i
		);

		return true;
	}
	return false;
}

bool Server::receiveFromCgi(size_t i)
{
	int fd = pollfds_vector[i].fd;

	Client *client = findClientByCgiFd(fd);

	if (!client)
		return false;

	char buffer[4096];

	ssize_t bytesRead = read(fd, buffer, sizeof(buffer));

	if (bytesRead > 0)
	{
		client->cgiResponse.append(buffer, bytesRead);
		return false;
	}
	if (bytesRead == -1)
	{
		close(fd);
		client->cgiOutputFd = -1;
		pollfds_vector.erase(pollfds_vector.begin() + i);
		handleError(*client, findLocation(*client), 500);
		client->cgiPid = -1;
		client->cgiStart = 0;
		return true;
	}

	/*
	 * bytesRead == 0
	 *
	 * CGI fechou stdout.
	 * Agora verificamos se o processo terminou normalmente
	 * ou se morreu com erro.
	 */

	int status;
	pid_t result = waitpid(
		client->cgiPid,
		&status,
		WNOHANG
	);

	if (result == -1)
	{
		std::cerr << "waitpid failed: "
				  << strerror(errno) << std::endl;

		close(fd);
		client->cgiOutputFd = -1;

		pollfds_vector.erase(
			pollfds_vector.begin() + i
		);

		handleError(*client, findLocation(*client), 500);
		client->cgiPid = -1;
		client->cgiStart = 0;

		return true;
	}

	if (result == client->cgiPid)
	{
		/*
		 * CGI terminou.
		 *
		 * Se terminou com exit code diferente de 0,
		 * consideramos erro CGI.
		 */
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		{
			std::cerr << "CGI process failed" << std::endl;

			close(fd);
			client->cgiOutputFd = -1;

			pollfds_vector.erase(
				pollfds_vector.begin() + i
			);

			handleError(*client, findLocation(*client), 500);
			client->cgiPid = -1;
			client->cgiStart = 0;

			return true;
		}
	}

	close(fd);
	client->cgiOutputFd = -1;

	pollfds_vector.erase(
		pollfds_vector.begin() + i
	);

	sendCGIResponse(*client, *this, client->cgiResponse);
	client->cgiPid = -1;
	client->cgiStart = 0;

	return true;
}
