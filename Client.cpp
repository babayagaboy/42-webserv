/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 13:57:27 by myivanov          #+#    #+#             */
/*   Updated: 2026/07/28 13:57:43 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client() {}

Client::Client(const Client &obj) : fd(obj.fd), bytes_read(obj.bytes_read), recvBuffer(obj.recvBuffer), request(obj.request) {}

Client& Client::operator=(const Client &obj) {
	if (this != &obj)
	{
		fd = obj.fd;
		bytes_read = obj.bytes_read;
		recvBuffer = obj.recvBuffer;
		request = obj.request;
	}
	return *this;
}

Client::~Client() {}