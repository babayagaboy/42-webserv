/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 13:54:38 by myivanov          #+#    #+#             */
/*   Updated: 2026/08/26 16:55:13 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "HTTPrequest.hpp"

class Client {
	public:
		int fd;
		int upstreamfd;
		int	cgiInputFd;
		int	cgiOutputFd;
		size_t	cgiBodyOffset;
		bool tunnel;
		bool newSession;
		std::string sessionId;

		size_t bytes_read;
		std::string recvBuffer;
		std::string	cgiBody;
		std::string cgiResponse;
		HTTPrequest request;
		pid_t		cgiPid;

		Client();
		Client(const Client &obj);
		Client& operator=(const Client &obj);
		~Client();
};

#endif