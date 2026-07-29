/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_partial_request.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/29 12:39:44 by myivanov          #+#    #+#             */
/*   Updated: 2026/07/29 23:01:03 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        std::cerr << "socket() failed\n";
        return 1;
    }

    sockaddr_in server = {};
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    if (connect(sock, (sockaddr *)&server, sizeof(server)) == -1)
    {
        std::cerr << "connect() failed\n";
        return 1;
    }

    std::string part1 =
        "POST /users HTTP/1.1\r\n"
        "Host: localhost\r\n";

    std::string part2 =
        "User-Agent: PartialTester\r\n"
        "Content-Length: 11\r\n"
        "\r\n"
        "Hello World";

    std::string request =
    "POST /users HTTP/1.1\r\n"
    "Host: localhost\r\n"
    "Content-Length: 11\r\n"
    "\r\n"
    "Hello World";

    for (size_t i = 0; i < request.size(); ++i)
    {
        send(sock, &request[i], 1, 0);
        usleep(100000); // 100 ms
    }

    std::cout << "Sent first half of the headers.\n";
    std::cout << "Sleeping for 3 seconds...\n";

    sleep(3);

    send(sock, part2.c_str(), part2.size(), 0);

    std::cout << "Sent the rest of the request.\n";

    close(sock);
}