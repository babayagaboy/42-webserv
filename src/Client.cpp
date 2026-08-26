/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 13:57:27 by myivanov          #+#    #+#             */
/*   Updated: 2026/08/26 15:49:01 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Client.hpp>

Client::Client()
    : fd(-1),
      upstreamfd(-1),
      cgiInputFd(-1),
      cgiOutputFd(-1),
      cgiBodyOffset(0),
      tunnel(false),
      bytes_read(0),
      recvBuffer(),
      cgiBody(),
      cgiResponse(),
      request(),
      sessionId(),
      newSession(false)
{
}

Client::Client(const Client &obj)
    : fd(obj.fd),
      upstreamfd(obj.upstreamfd),
      cgiInputFd(obj.cgiInputFd),
      cgiOutputFd(obj.cgiOutputFd),
      cgiBodyOffset(obj.cgiBodyOffset),
      tunnel(obj.tunnel),
      bytes_read(obj.bytes_read),
      recvBuffer(obj.recvBuffer),
      cgiBody(obj.cgiBody),
      cgiResponse(obj.cgiResponse),
      request(obj.request),
      sessionId(obj.sessionId),
      newSession(obj.newSession)
{
}

Client& Client::operator=(const Client &obj) {
    if (this != &obj)
    {
        fd = obj.fd;
        upstreamfd = obj.upstreamfd;
        cgiInputFd = obj.cgiInputFd;
        cgiOutputFd = obj.cgiOutputFd;
        cgiBodyOffset = obj.cgiBodyOffset;
        tunnel = obj.tunnel;
        bytes_read = obj.bytes_read;
        recvBuffer = obj.recvBuffer;
        cgiBody = obj.cgiBody;
        cgiResponse = obj.cgiResponse;
        request = obj.request;
        sessionId = obj.sessionId;
        newSession = obj.newSession;
    }
    return *this;
}

Client::~Client() {}