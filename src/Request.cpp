/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 14:19:37 by hgutterr          #+#    #+#             */
/*   Updated: 2026/08/10 17:54:34 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Server.hpp>

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

int	method_GET()
{
	;
}

int	method_POST()
{
	;
}

int	method_DELETE()
{
	;
}

int	method_PUT()
{
	;
}

int	method_HEAD()
{
	;
}

int	method_OPTIONS()
{
	;
}

int	method_TRACE()
{
	;
}

int	method_CONNECT()
{
	;
}

int	method_PATCH()
{
	;
}

void	processRequest(Client &c)
{

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
	
	int (*methfunctions[]) ( void ) = {
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
			methfunctions[i]();
	}
	
	print_info(c.request);
}