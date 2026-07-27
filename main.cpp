/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 14:51:28 by myivanov          #+#    #+#             */
/*   Updated: 2026/07/27 12:03:57 by myivanov         ###   ########.fr       */
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

int	main()
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
    std::cout << "Client Socket FD is : " << clientSocket << std::endl;

    char buff[1024] = {};

    ssize_t message_len = recv(clientSocket, &buff, 200, 0);

    HTTPrequest request;
    //std::stringstream ss(buff);
    
    //std::getline(ss, firstLine);
    
    std::string firstLine(buff);
    std::stringstream lineStream(firstLine);

    lineStream >> request.method;
    lineStream >> request.content;
    lineStream >> request.version;



    std::cout << "Message len recieved was: " << message_len << std::endl;
    std::cout << "Message: " << buff;

    std::cout << std::endl << "HTTPrequest method: " << request.method  << std::endl;
    std::cout << "HTTPrequest content: " << request.content  << std::endl;
    std::cout << "HTTPrequest version: " << request.version  << std::endl;
    
    return 0;
}

