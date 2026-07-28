/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fill_HTTP_object.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 15:19:33 by myivanov          #+#    #+#             */
/*   Updated: 2026/07/28 14:57:53 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPrequest.hpp"

void    rev_request_firstLine(HTTPrequest &obj, std::stringstream &ss)
{
	std::string firstLine;
	std::getline(ss, firstLine);
	std::stringstream lineStream(firstLine);

	lineStream >> obj.method;
	lineStream >> obj.path;
	lineStream >> obj.version;
}

void    rev_request_body(HTTPrequest &obj, std::stringstream &ss)
{
	
	std::string body_text;

	while (std::getline(ss, body_text)) {
		obj.body.append(body_text);
	}
}

void    rev_request_hosts(HTTPrequest &obj, std::stringstream &ss)
{
	std::string		header;
	size_t			iterator;

	while (std::getline(ss, header))
	{
		if (header.empty() || header == "\r")
			break ;
		iterator = header.find(":");
		if (iterator != std::string::npos) {
			std::string header_content = header.substr(iterator);
			obj.headers.insert(std::make_pair(header.substr(0, iterator), header_content));
		}
	}
	rev_request_body(obj, ss);
}

HTTPrequest fill_HTTP_object(std::stringstream &ss) {
	HTTPrequest obj;

	rev_request_firstLine(obj, ss);
	rev_request_hosts(obj, ss);

	return obj;
}