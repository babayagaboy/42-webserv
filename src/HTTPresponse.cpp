/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPresponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 16:12:25 by myivanov          #+#    #+#             */
/*   Updated: 2026/08/19 15:09:30 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <HTTPresponse.hpp>

HTTPresponse::HTTPresponse() : _body("") {}

HTTPresponse::HTTPresponse(const HTTPresponse &obj) : _statusCode(obj._statusCode), _headers(obj._headers), _body(obj._body) {}

HTTPresponse& HTTPresponse::operator=(const HTTPresponse &obj) {
	if (this != &obj)
	{
		_body = obj._body;
		_headers = obj._headers;
		_body = obj._body;
	}
	return *this;
}

HTTPresponse::~HTTPresponse() {}

const std::string& HTTPresponse::getBody() const
{
	return _body;
}

const std::vector<std::pair<std::string, std::string> >& HTTPresponse::getHeaders() const
{
	return _headers;
}

int		HTTPresponse::getStatusCode() {
	return _statusCode;
}

std::string getStatusMessage(int status)
{
	switch (status)
	{
		case 200: return "OK";
		case 201: return "Created";
		case 204: return "No Content";
		case 301: return "Moved Permanently";
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 500: return "Internal Server Error";
		default:  return "Unknown";
	}
}

void    HTTPresponse::setStatusCode(int code)
{
	_statusCode = code;
}

void    HTTPresponse::setHeaders(const std::vector<std::pair<std::string, std::string> >&head)
{
	_headers = head;
}

void	HTTPresponse::setBody(const std::string &body)
{
	_body = body;
}

std::string HTTPresponse::buildResponse() const
{
	std::stringstream responseStr;

	responseStr	<< "HTTP/1.1 "
				<< _statusCode
				<< " "
				<< getStatusMessage(_statusCode)
				<< "\r\n";

	for (size_t i = 0; i < _headers.size(); ++i)
	{
		responseStr	<< _headers[i].first
					<< ": "
					<< _headers[i].second
					<< "\r\n";
	}

	responseStr << "\r\n";
	responseStr << _body;

	return (responseStr.str());
}