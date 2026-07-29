/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 13:54:38 by myivanov          #+#    #+#             */
/*   Updated: 2026/07/28 13:57:40 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "HTTPrequest.hpp"

class Client {
	public:
		int fd;
		size_t bytes_read;
		std::string recvBuffer;
		HTTPrequest request;

		Client();
		Client(const Client &obj);
		Client& operator=(const Client &obj);
		~Client();
};

#endif