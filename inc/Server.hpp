/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 16:14:30 by myivanov          #+#    #+#             */
/*   Updated: 2026/08/06 16:24:11 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <HTTPrequest.hpp>
# include <ServerConf.hpp>
# include <Client.hpp>
# include <poll.h>
# include <cstring>
# include <vector>
# include <unistd.h>
# include <fstream>
# include <algorithm>
# include <string>

class Server
{
	public:
		int serverSocket;
		sockaddr_in socketAddress;
		socklen_t address_size;

		std::vector<pollfd> pollfds_vector;
		std::map<int, Client> clients;

		std::vector<ServerConf> serversConfs;

		void acceptNewClient();
		bool receiveFromClient(size_t i);
		void disconnectClient(size_t i);

		void	run();

		Server(int fd, sockaddr_in addr, std::vector<pollfd> &pollfds, std::map<int, Client> &clientMap);
};

struct Counter {
    int listenCounter;
    int hostCounter;
    int clientMaxCounter;
    int serverNameCounter;
};

struct CounterLocation {
    int returnCounter;
    int rootCounter;
    int indexCounter;
    int autoIndexCounter;
};

int fillServerConfig(char *confFileName);

#endif