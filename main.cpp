/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 14:51:28 by myivanov          #+#    #+#             */
/*   Updated: 2026/07/28 13:07:28 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPrequest.hpp"
#include <poll.h>
#include <vector>
#include <unistd.h>
#include <algorithm>

void        rev_request_firstLine(HTTPrequest &obj, std::stringstream &ss);
void        rev_request_body(HTTPrequest &obj, std::stringstream &ss);
void        rev_request_hosts(HTTPrequest &obj, std::stringstream &ss);
HTTPrequest fill_HTTP_object(std::stringstream &ss);

void    print_info(const HTTPrequest &obj) {

    std::cout << std::endl << "HTTPrequest method: " << obj.method  << std::endl;
    std::cout << "HTTPrequest content: " << obj.path  << std::endl;
    std::cout << "HTTPrequest version: " << obj.version  << std::endl << std::endl;

    std::map<std::string, std::string>::const_iterator it;

    for (it = obj.headers.begin(); it != obj.headers.end(); ++it) {
        std::cout << it->first << it->second << std::endl;
    }

    std::cout << "Body: " << obj.body << std::endl;
}


int create_server_socket() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == -1) {
        std::cout << "Error\nServer socket FD is " << serverSocket << std::endl;
        return -1;
    }
    return serverSocket;
}


void	configureSocketAddress(struct sockaddr_in &socketAddress)
{
	socketAddress.sin_family = AF_INET;
    socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    socketAddress.sin_port = htons(8080);
}



int 	main()
{
    int serverSocket = create_server_socket();
    if (serverSocket == -1)
		return -1;

    struct sockaddr_in socketAddress {};
	configureSocketAddress(socketAddress);

    if (bind(serverSocket, (struct sockaddr *)&socketAddress, sizeof(socketAddress)) == -1)
    {
        std::cout << "Error\nCould not bind socket fd to its address" << std::endl;
        return -1;
    }

    if (listen(serverSocket, 120) == -1)
    {
        std::cout << "Error\nCould not change server status to listen mode" << std::endl;
        return -1;
    }

    socklen_t address_size = sizeof(socketAddress);
	struct pollfd serverPollFd {};
	serverPollFd.fd = serverSocket;
	serverPollFd.events = POLLIN;

	std::vector<pollfd> pollfds_vector{};

	pollfds_vector.push_back(serverPollFd);

	std::cout << "Server is now listening..." << std::endl;

    while (true) {

		if (poll(pollfds_vector.data(), pollfds_vector.size(), -1) == -1)
			return -1;

		for (size_t i {}; i < pollfds_vector.size(); ++i) {
			int clientSocket;
			if (pollfds_vector[i].fd == serverSocket && pollfds_vector[i].revents & POLLIN) {
				clientSocket =  accept(serverSocket, (struct sockaddr *)&socketAddress, &address_size);
				if (clientSocket == -1) {
					std::cout << "Failed to accept incoming connection. No valid client socket fd was created" << std::endl;
					return -1;
				}
				pollfd clientPollFd {};
				clientPollFd.fd = clientSocket;
				clientPollFd.events = POLLIN;

				pollfds_vector.push_back(clientPollFd);
			}
			else if (pollfds_vector[i].fd != serverSocket && pollfds_vector[i].revents & POLLIN) {
				char buff[1024] = {};
				ssize_t message_len = recv(pollfds_vector[i].fd, &buff, 500, 0);
				if (message_len == 0)
				{
					close(pollfds_vector[i].fd);
					pollfds_vector.erase(pollfds_vector.begin() + i);
					--i;
					continue ;
				}

				std::stringstream ss(buff);
				HTTPrequest request = fill_HTTP_object(ss);
				print_info(request);
				std::cout << std::endl << std::endl;

			}
		}
    }

    
    return 0;
}

