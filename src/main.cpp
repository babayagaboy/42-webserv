/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 14:51:28 by myivanov          #+#    #+#             */
/*   Updated: 2026/08/09 13:11:14 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <HTTPrequest.hpp>
#include <Client.hpp>
#include <Server.hpp>

#include <poll.h>
#include <vector>
#include <unistd.h>
#include <algorithm>

void        rev_request_firstLine(HTTPrequest &obj, std::stringstream &ss);
void        rev_request_body(HTTPrequest &obj, std::stringstream &ss);
void        rev_request_hosts(HTTPrequest &obj, std::stringstream &ss);
HTTPrequest fill_HTTP_object(std::stringstream &ss);
int         parseConfigFile(char *configFilename);

void    print_info(const HTTPrequest &obj) {

    std::cout << std::endl << "HTTPrequest method: " << obj.method  << std::endl;
    std::cout << "HTTPrequest content: " << obj.path  << std::endl;
    std::cout << "HTTPrequest version: " << obj.version  << std::endl << std::endl;

    std::map<std::string, std::string>::const_iterator it;

    for (it = obj.headers.begin(); it != obj.headers.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
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
    socketAddress.sin_port = htons(8081);
}


int main(int ac, char **av)
{
	if(ac != 2)
		return 1;
    
    Server server;
	if (!fillServerConfig(av[1], server))
        return -1;

    server.serverSocket = create_server_socket();
    if (server.serverSocket == -1)
        return -1;

    sockaddr_in socketAddress = {};
    configureSocketAddress(socketAddress);

    if (bind(server.serverSocket, (struct sockaddr *)&socketAddress, sizeof(socketAddress)) == -1)
        return -1;

    if (listen(server.serverSocket, 120) == -1)
        return -1;

    struct pollfd serverPollFd = {};
    serverPollFd.fd = server.serverSocket;
    serverPollFd.events = POLLIN;

    std::vector<pollfd> pollfds_vector;
    pollfds_vector.push_back(serverPollFd);

    std::map<int, Client> clients;
    //Server server(server.serverSocket, socketAddress, pollfds_vector, clients);

    std::cout << "Server is now listening..." << std::endl;
    server.run();

    return 0;
}

