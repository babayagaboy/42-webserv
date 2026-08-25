/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 13:57:27 by myivanov          #+#    #+#             */
/*   Updated: 2026/08/25 17:19:59 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Client.hpp>

Client::Client() : fd(-1), upstreamfd(-1), cgiInputFd(-1), cgiOutputFd(-1), tunnel(false), bytes_read(0)
{
}

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