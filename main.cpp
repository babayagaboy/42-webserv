/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 14:51:28 by myivanov          #+#    #+#             */
/*   Updated: 2026/07/27 13:15:36 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>
#include <sstream>

class HTTPrequest {
    public:
        std::string method;
        std::string content;
        std::string version;

        std::map<std::string, std::string> hosts;

        std::string body;
        
        HTTPrequest();
        HTTPrequest(const HTTPrequest &obj);
        HTTPrequest& operator=(const HTTPrequest &obj);
        ~HTTPrequest();
};

HTTPrequest::HTTPrequest() {}

HTTPrequest::HTTPrequest(const HTTPrequest &obj) : method(obj.method), content(obj.content), version(obj.version),
                                                   hosts(obj.hosts), body(obj.body){}

HTTPrequest& HTTPrequest::operator=(const HTTPrequest &obj) {
    if (this != &obj) {
        method = obj.method;
        content = obj.content;
        version = obj.version;
        hosts = obj.hosts;
        body = obj.body;
    }
    return *this;
}

HTTPrequest::~HTTPrequest() {}


void    rev_request(HTTPrequest &obj, const std::string &first_line) {
    std::stringstream lineStream(first_line);

    lineStream >> obj.method;
    lineStream >> obj.content;
    lineStream >> obj.version;
}

void    rev_request_hosts(HTTPrequest &obj, std::stringstream &ss) {
    std::string hosts;

    while (getline(ss, hosts, ':')) {
        std::string host_content;
        getline(ss, host_content);
        obj.hosts.insert(std::make_pair(hosts, host_content));
    }
}


void    print_info(const HTTPrequest &obj) {

    std::cout << std::endl << "HTTPrequest method: " << obj.method  << std::endl;
    std::cout << "HTTPrequest content: " << obj.content  << std::endl;
    std::cout << "HTTPrequest version: " << obj.version  << std::endl << std::endl;

    std::cout << "HTTPrequest hosts:" << std::endl;

    std::map<std::string, std::string>::const_iterator it;

    for (it = obj.hosts.begin(); it != obj.hosts.end(); ++it) {
        std::cout << it->first << ":" << it->second << std::endl;
    }
}

int 	main()
{
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

    ssize_t message_len = recv(clientSocket, &buff, 200, 0);

    (void)message_len;
    HTTPrequest request;

    std::stringstream ss(buff);

    std::string firstLine;
    std::getline(ss, firstLine);
    
    rev_request(request, firstLine);
    rev_request_hosts(request, ss);


    print_info(request);
    
    return 0;
}

