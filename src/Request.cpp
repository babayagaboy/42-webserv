/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 14:19:37 by hgutterr          #+#    #+#             */
/*   Updated: 2026/08/12 17:34:35 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <HTTPrequest.hpp>
#include <HTTPresponse.hpp>
#include <Server.hpp>


// HTTP/1.1 200 OK\r\n
// Content-Type: text/html; charset=UTF-8\r\n
// Date: Fri, 21 Jun 2024 14:18:33 GMT\r\n
// Last-Modified: Thu, 17 Oct 2019 07:18:26 GMT\r\n
// Content-Length: 1234\r\n
// \r\n

void    print_info(const HTTPrequest &obj)
{
	std::cout << std::endl << "HTTPrequest method: " << obj.method  << std::endl;
	std::cout << "HTTPrequest content: " << obj.path  << std::endl;
	std::cout << "HTTPrequest version: " << obj.version  << std::endl << std::endl;

	std::map<std::string, std::string>::const_iterator it;

	for (it = obj.headers.begin(); it != obj.headers.end(); ++it) {
		std::cout << it->first << ": " << it->second << std::endl;
	}

	std::cout << "Body: " << obj.body << std::endl;
}



int	method_GET( const Client &c, const Server &s, int l )
{
	HTTPresponse response;
	Location location = s.serversConfs.getLocations()[l];
	std::string path(location.getPagePath());
	
	std::cout << "PATH: " << path << std::endl;
	int fd = open(path.c_str(), O_RDONLY);
	std::cout << "good old goy2" << std::endl;

	if (fd < 0)
		return 0;

	char buffer[4096];
	std::string body;
	ssize_t bytesRead;

	std::cout << "good old goy1" << std::endl;

	while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
		body.append(buffer, bytesRead);
	close(fd);

	std::vector<std::pair<std::string, std::string> > headers;

	std::stringstream ss;

	ss << body.size();
	std::string bodySize = ss.str();

	headers.push_back(std::make_pair("Content-Length", bodySize));
	headers.push_back(std::make_pair("Content-Type", "text/html"));

	
	response.setStatusCode(200);
	response.setBody(body);
	response.setHeaders(headers);
	
	std::string responseStr = response.buildResponse();
	
	std::cout << "good old goy" << std::endl;
	send(c.fd, responseStr.c_str(), responseStr.size(), 0);
}

int	method_POST( const Client &c, const Server &s, int l )
{
	;
}

int	method_DELETE( const Client &c, const Server &s, int l )
{
	;
}

int	method_PUT( const Client &c, const Server &s, int l )
{
	;
}

int	method_HEAD( const Client &c, const Server &s, int l )
{
	;
}

int	method_OPTIONS( const Client &c, const Server &s, int l )
{
	;
}

int	method_TRACE( const Client &c, const Server &s, int l )
{
	;
}

int	method_CONNECT( const Client &c, const Server &s, int l )
{
	;
}

int	method_PATCH( const Client &c, const Server &s, int l )
{
	;
}


void	processRequest(const Client &c, const Server &s)
{
	int location;

	std::string methods[] = {
		"GET",
		"POST",
		"DELETE",
		"PUT",
		"HEAD",
		"OPTIONS",
		"TRACE",
		"CONNECT",
		"PATCH"
	};
	
	int (*methfunctions[]) ( const Client &c, const Server &s, int location ) = {
		&method_GET, 
		&method_POST, 
		&method_DELETE,
        &method_PUT,
		&method_HEAD,
		&method_OPTIONS,
		&method_TRACE,
		&method_CONNECT,
		&method_PATCH
	};

	for (size_t i = 0; i < methods->size(); ++i)
	{
		if(c.request.method == methods[i])
		{
			location = s.verifyAllowedMethods(c);
			if (location < 0)
			{
				std::cout << "good old goywwwwwww" << std::endl;
				return ;
			}
			std::cout << "HIIII" << std::endl;
			methfunctions[i](c, s, location);
		}
	}
	
	print_info(c.request);
}