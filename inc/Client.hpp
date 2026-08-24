/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 13:54:38 by myivanov          #+#    #+#             */
/*   Updated: 2026/08/24 21:29:53 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "HTTPrequest.hpp"

class Client {
	public:
		int fd;
		int upstreamfd;
		bool tunnel;
		size_t bytes_read;
		std::string recvBuffer;
		HTTPrequest request;

		Client();
		Client(const Client &obj);
		Client& operator=(const Client &obj);
		~Client();
};

#endif