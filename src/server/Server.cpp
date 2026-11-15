/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 16:16:22 by myivanov          #+#    #+#             */
/*   Updated: 2026/11/15 17:40:26 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Server.hpp>
#include <cstring>
#include <iostream>
#include <cerrno>


volatile sig_atomic_t g_running = 1;

void signalHandler(int sig)
{
    (void)sig;
    g_running = 0;
}

Server::Server() : sessionCounter(0) {
	address_size = sizeof(socketAddress);
}

Server::Server(int fd, sockaddr_in addr, std::vector<pollfd> &pollfds, std::map<int, Client> &clientMap) {
	serverSocket = fd;
	socketAddress = addr;
	pollfds_vector = pollfds;
	address_size = sizeof(socketAddress);
	clients = clientMap;
}

void Server::setPeerServers(const std::vector<Server *> &peers)
{
	peerServers = peers;
}

int Server::getSocket() { return this->serverSocket; }

void Server::setServerId( int i ) { this->serverid = i; }

int Server::getServerId() const { return this->serverid; }

void Server::run()
{
    while (g_running)
    {
		std::vector<pollfd> readyPollfds;
		std::vector<Server *> owners;

		std::vector<Server *> activeServers = peerServers;
		if (activeServers.empty())
			activeServers.push_back(this);

		for (size_t serverIndex = 0; serverIndex < activeServers.size(); ++serverIndex)
		{
			Server *server = activeServers[serverIndex];
			for (size_t fdIndex = 0; fdIndex < server->pollfds_vector.size(); ++fdIndex)
			{
				readyPollfds.push_back(server->pollfds_vector[fdIndex]);
				owners.push_back(server);
			}
		}

		for (size_t serverIndex = 0; serverIndex < activeServers.size(); ++serverIndex)
			activeServers[serverIndex]->cleanupExpiredCgi();

		int ret = poll(
			readyPollfds.data(),
			readyPollfds.size(),
			1000
        );

		if (ret == -1)
		{
			if (errno == EINTR)
				continue;
		
			std::cerr << "poll failed: "
					  << strerror(errno) << std::endl;
			return;
		}

		if (!g_running)
			break;

		for (size_t i = 0; i < readyPollfds.size(); ++i)
        {
			if (readyPollfds[i].revents == 0)
                continue;

			Server *owner = owners[i];
			int fd = readyPollfds[i].fd;
			short events = readyPollfds[i].revents;
			size_t localIndex = 0;
			while (localIndex < owner->pollfds_vector.size()
				&& owner->pollfds_vector[localIndex].fd != fd)
				++localIndex;
			if (localIndex == owner->pollfds_vector.size())
				continue;

			if (fd == owner->serverSocket)
            {
                if (events & POLLIN)
					owner->acceptNewClient();
                continue;
            }

			if (owner->isCgiOutputFd(fd))
            {
                if (events & (POLLIN | POLLHUP | POLLERR))
                {
					bool removed = owner->receiveFromCgi(localIndex);
                    if (removed)
                        continue;
                }
                continue;
            }

			if (owner->isCgiInputFd(fd))
            {
                if (events & (POLLOUT | POLLERR | POLLHUP))
                {
					bool removed = owner->sendToCgi(localIndex);
                    if (removed)
                        continue;
                }
                continue;
            }

            if (events & POLLOUT)
            {
				bool removed = owner->sendToClient(localIndex);
                if (removed)
                    continue;
            }

            if (events & (POLLIN | POLLHUP | POLLERR))
            {
				bool removed = owner->receiveFromClient(localIndex);
                if (removed)
                    continue;
            }
        }
		
    }
}
