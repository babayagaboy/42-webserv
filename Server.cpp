/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 16:16:22 by myivanov          #+#    #+#             */
/*   Updated: 2026/07/29 13:03:47 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

HTTPrequest fill_HTTP_object(std::stringstream &ss);
void    print_info(const HTTPrequest &obj);

Server::Server(int fd, sockaddr_in addr, std::vector<pollfd> &pollfds, std::map<int, Client> &clientMap) {
    serverSocket = fd;
    socketAddress = addr;
    pollfds_vector = pollfds;
    address_size = sizeof(socketAddress);
    clients = clientMap;
}

void Server::acceptNewClient() {
	int clientFd =  accept(serverSocket, (struct sockaddr *)&socketAddress, &address_size);
	if (clientFd == -1) {
		std::cout << "Failed to accept incoming connection. No valid client socket fd was created" << std::endl;
		return ;
	}
	clients[clientFd] = Client();
	clients[clientFd].fd = clientFd;

	pollfd clientPollFd {};
	clientPollFd.fd = clientFd;
	clientPollFd.events = POLLIN;

	pollfds_vector.push_back(clientPollFd);
}

bool Server::receiveFromClient(size_t i)
{
    int clientFd = pollfds_vector[i].fd;
    Client &client = clients[clientFd];
    char buff[4096] = {};

    client.bytes_read = recv(client.fd, buff, sizeof(buff), 0);
    if (client.bytes_read == static_cast<size_t>(-1)) {
        std::cout << "recv() failed" << std::endl;
        return (disconnectClient(i), true);
    }

    if (client.bytes_read == 0)
        return (disconnectClient(i), true);

    client.recvBuffer.append(buff, client.bytes_read);

    if (client.recvBuffer.find("\r\n\r\n") == std::string::npos)
        return false;

    std::stringstream ss(client.recvBuffer);
    client.request = fill_HTTP_object(ss);

    if (client.request.headers.find("Content-Length") != client.request.headers.end()) {
        size_t headerEnd = client.recvBuffer.find("\r\n\r\n");
        size_t bodyStart = headerEnd + 4;

        size_t receivedBodySize = client.recvBuffer.size() - bodyStart;

        std::ostringstream oss;
        oss << receivedBodySize;

        if (client.request.headers["Content-Length"] != oss.str())
            return false;
    }
    print_info(client.request);
    std::cout << std::endl << std::endl;
    return false;
}

void Server::disconnectClient(size_t i)
{
    int clientFd = pollfds_vector[i].fd;

    close(clientFd);

    clients.erase(clientFd);

    pollfds_vector.erase(pollfds_vector.begin() + i);
}

void Server::run()
{
    while (true)
    {
        if (poll(pollfds_vector.data(), pollfds_vector.size(), -1) == -1)
            return;

        for (size_t i = 0; i < pollfds_vector.size(); ++i)
        {
            if (pollfds_vector[i].fd == serverSocket && (pollfds_vector[i].revents & POLLIN)) {
                acceptNewClient();
            }
            else if (pollfds_vector[i].revents & POLLIN) {
                if (receiveFromClient(i)) {
                    --i;
                    continue;
                }
            }
        }
    }
}