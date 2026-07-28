/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 16:14:30 by myivanov          #+#    #+#             */
/*   Updated: 2026/07/28 16:24:23 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "HTTPrequest.hpp"
# include "Client.hpp"
# include <poll.h>
# include <vector>
# include <unistd.h>
# include <algorithm>

class Server
{
	public:
		int serverSocket;
		sockaddr_in socketAddress;
		socklen_t address_size;

		std::vector<pollfd> pollfds_vector;
		std::map<int, Client> clients;

		void acceptNewClient();
		bool receiveFromClient(size_t i);
		void disconnectClient(size_t i);

		void	run();

		Server(int fd, sockaddr_in addr, std::vector<pollfd> &pollfds, std::map<int, Client> &clientMap);
};

#endif