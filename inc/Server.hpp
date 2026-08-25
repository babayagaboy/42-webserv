/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 16:14:30 by myivanov          #+#    #+#             */
/*   Updated: 2026/08/25 13:41:35 by myivanov         ###   ########.fr       */
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
		int						serverSocket;
		int						serverid;
		sockaddr_in				socketAddress;
		socklen_t				address_size;
	
		std::vector<pollfd>		pollfds_vector;
		std::map<int, Client>	clients;

		ServerConf				serversConfs;

		void	acceptNewClient();
		bool	receiveFromClient(size_t i);
		bool	receiveFromCgi(size_t i);
		void	receiveFromUpstream( size_t i );
		void	disconnectClient(size_t i);
		int		getSocket();
		void	setServerId( int i );
		int		getServerId() const;
		bool	isUpstreamFd(int fd) const;
		bool	isCgiOutputFd(int fd) const;
		bool	isCgiInputFd(int fd) const;
		bool	sendToCgi(size_t i);
		Client* findClientByUpstreamFd(int fd);
		Client* findClientByCgiFd(int fd);
		
		int		findLocation( const Client& c ) const;
		bool 	isMethodAllowed( const std::string &method, int l ) const;
		void	run();

		Server();
		Server(int fd, sockaddr_in addr, std::vector<pollfd> &pollfds, std::map<int, Client> &clientMap);
};


struct Counter {
    int listenCounter;
    int hostCounter;
    int clientMaxCounter;
    int serverNameCounter;

	Counter();
};

struct CounterLocation {
    int returnCounter;
    int rootCounter;
    int indexCounter;
    int autoIndexCounter;
	int allowedCounter;

	CounterLocation();
};

int fillServerConfig(char *confFileName, std::vector<Server> &server);

#endif