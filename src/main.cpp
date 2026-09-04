/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mykytaivanov <mykytaivanov@student.42.f    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 14:51:28 by myivanov          #+#    #+#             */
/*   Updated: 2026/09/04 16:54:45 by mykytaivano      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <HTTPrequest.hpp>
#include <Client.hpp>
#include <Server.hpp>

#include <poll.h>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <algorithm>
#include <netdb.h>
#include <signal.h>

void        rev_request_firstLine(HTTPrequest &obj, std::stringstream &ss);
void        rev_request_body(HTTPrequest &obj, std::stringstream &ss);
void        rev_request_hosts(HTTPrequest &obj, std::stringstream &ss);
HTTPrequest fill_HTTP_object(std::stringstream &ss);
int         parseConfigFile(char *configFilename);

int create_server_socket() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cout << "Error\nServer socket FD is " << serverSocket << std::endl;
        return -1;
    }

    int opt = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		std::cerr << "setsockopt(SO_REUSEADDR) failed: " << strerror(errno) << std::endl;
		close(serverSocket);
		return -1;
	}

    return serverSocket;
}


void	configureSocketAddress(Server &s)
{
	std::memset(&s.socketAddress, 0, sizeof(s.socketAddress));
	s.socketAddress.sin_family = AF_INET;
    s.socketAddress.sin_port = htons(s.serversConfs.getListenPort());
	
	struct addrinfo hints;
	struct addrinfo *result;

	std::memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(s.serversConfs.getHost().c_str(), NULL, &hints, &result) != 0) {
		std::cout << "getaddrinfo failed" << std::endl;
		return;
	}

	s.socketAddress.sin_addr = ((struct sockaddr_in *)result->ai_addr)->sin_addr;

	freeaddrinfo(result);
}


int main(int ac, char **av)
{
	if(ac != 2)
	{
		// loaddefault();
		return 1;
	}

	std::vector<Server> servers;
	if (!fillServerConfig(av[1], servers))
		return -1;

    std::cout << "Server is now listening..." << std::endl;
	signal(SIGPIPE, SIG_IGN);
	while (true)
	{
		for (size_t i = 0; i < servers.size(); ++i) {
			servers[i].serverSocket = create_server_socket();
			configureSocketAddress(servers[i]);

			if (bind(servers[i].serverSocket, (struct sockaddr *)&servers[i].socketAddress, sizeof(servers[i].socketAddress)) == -1) {
				std::cerr << "bind failed: " << strerror(errno) << std::endl;
				return -1;
			}
			if (listen(servers[i].serverSocket, 120) == -1)
				return -1;

			pollfd serverPollFd = {};
			serverPollFd.fd = servers[i].serverSocket;
			serverPollFd.events = POLLIN;
		
			servers[i].pollfds_vector.push_back(serverPollFd);

			pid_t pid = fork();
			if (pid < 0) {
				perror("fork");
				return 1;
			}
			if (pid == 0)
			{
				servers[i].setServerId(i);
				std::cerr << "Starting server " << i
						<< " PID=" << getpid() << std::endl;

				std::cerr << "BEFORE run() server " << i
						<< " PID=" << getpid() << std::endl;

				servers[i].run();

				std::cerr << "AFTER run() server " << i
						<< " PID=" << getpid() << std::endl;

				std::cerr << "WARNING: server " << i
						<< " run() RETURNED! PID=" << getpid() << std::endl;

				_exit(0);
			}
			else 
				close(servers[i].serverSocket);
		}

		while (wait(NULL) > 0)
			;
	}
    return 0;
}

