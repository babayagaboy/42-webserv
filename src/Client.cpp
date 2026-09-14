/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 13:57:27 by myivanov          #+#    #+#             */
/*   Updated: 2026/09/14 14:53:34 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Client.hpp>

Client::Client()
        : fd(-1),
        cgiInputFd(-1),
        cgiOutputFd(-1),
        cgiBodyOffset(0),
        newSession(false),
        sessionId(""),
        bytes_read(0),
        recvBuffer(""),
        cgiBody(""),
        cgiResponse(""),
        request(),
        cgiPid(),
        cgiStart(0),
        sendBuffer(),
        sendOffset(0)

{
}

Client::Client(const Client &obj)
    : fd(obj.fd),
      cgiInputFd(obj.cgiInputFd),
      cgiOutputFd(obj.cgiOutputFd),
      cgiBodyOffset(obj.cgiBodyOffset),
      newSession(obj.newSession),
      sessionId(obj.sessionId),
      bytes_read(obj.bytes_read),
      recvBuffer(obj.recvBuffer),
      cgiBody(obj.cgiBody),
      cgiResponse(obj.cgiResponse),
      request(obj.request),
      cgiPid(obj.cgiPid),
    cgiStart(obj.cgiStart),
      sendBuffer(obj.sendBuffer),
	  sendOffset(obj.sendOffset)
{
}

Client& Client::operator=(const Client &obj) {
    if (this != &obj)
    {
        fd = obj.fd;
        cgiInputFd = obj.cgiInputFd;
        cgiOutputFd = obj.cgiOutputFd;
        cgiBodyOffset = obj.cgiBodyOffset;
        bytes_read = obj.bytes_read;
        recvBuffer = obj.recvBuffer;
        cgiBody = obj.cgiBody;
        cgiResponse = obj.cgiResponse;
        request = obj.request;
        sessionId = obj.sessionId;
        newSession = obj.newSession;
        cgiPid = obj.cgiPid;
        cgiStart = obj.cgiStart;
        sendBuffer = obj.sendBuffer;
        sendOffset = obj.sendOffset;
    }
    return *this;
}

Client::~Client() {}