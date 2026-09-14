/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 13:54:38 by myivanov          #+#    #+#             */
/*   Updated: 2026/09/14 14:53:34 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "HTTPrequest.hpp"
# include <ctime>

class Client {
	public:
		int fd;
		int	cgiInputFd;
		int	cgiOutputFd;
		size_t	cgiBodyOffset;
		bool newSession;
		std::string sessionId;

				ssize_t bytes_read;
		std::string recvBuffer;
		std::string	cgiBody;
		std::string cgiResponse;
		HTTPrequest request;
		pid_t		cgiPid;
		time_t  cgiStart;

		std::string sendBuffer;
		size_t sendOffset;

		Client();
		Client(const Client &obj);
		Client& operator=(const Client &obj);
		~Client();
};

#endif