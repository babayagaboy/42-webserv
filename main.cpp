/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 14:51:28 by myivanov          #+#    #+#             */
/*   Updated: 2026/07/27 18:14:01 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPrequest.hpp"
#include "server.hpp"

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

    std::cout << "Body:" << obj.body << std::endl;
}

int 	main(int ac, char **av)
{
	if(ac != 2)
		return ;
	fillServerConfig(av[1]);

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == -1)
    {
        std::cout << "Error\nServer socket FD is " << serverSocket << std::endl;
        return -1;
    }

    struct sockaddr_in socketAddress = {};

    socketAddress.sin_family = AF_INET;
    socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    socketAddress.sin_port = htons(8080);

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

    std::cout << "Server is now listening..." << std::endl;

    socklen_t address_size = sizeof(socketAddress);
    
    int clientSocket = accept(serverSocket, (struct sockaddr *)&socketAddress, &address_size);

    if (clientSocket == -1)
    {
        std::cout << "Failed to accept incoming connection. No valid client socket fd was created" << std::endl;
        return -1;
    }

    std::cout << "Conection established successfully!!" << std::endl;

    std::cout << "Server FD is : " << serverSocket << std::endl;
    std::cout << "Client Socket FD is : " << clientSocket << std::endl << std::endl;

    char buff[1024] = {};

    ssize_t message_len = recv(clientSocket, &buff, 500, 0);

    (void)message_len;

    std::stringstream ss(buff);
    HTTPrequest request = fill_HTTP_object(ss);

    print_info(request);
    
    return 0;
}

