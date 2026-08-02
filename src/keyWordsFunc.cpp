/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   keyWordsFunc.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 17:33:26 by hgutterr          #+#    #+#             */
/*   Updated: 2026/08/02 18:15:23 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Server.hpp>
#include <iostream>

void	check_ipAdress(const std::string &token)
{
	std::string line;
	double		num;
	int			range;

	while ((line = std::strstr(token.c_str(), ".")) )
	{
		num = std::strtod(tokens[i + 1].c_str(), NULL);
		
	}
}

int handle_listen( std::vector<std::string> &tokens, int i )
{
	int listenPort;
	double	num;

	std::cout << "token = listen, ";
	std::cout << "i = " << i << "\n";
	
	num = std::strtod(tokens[i + 1].c_str(), NULL);
	listenPort = static_cast<int>(num);
	if (listenPort != num)
		return (0);
	if (tokens[i + 2] != ";")
		return (0);
	return(2);
}

int handle_host( std::vector<std::string> &tokens, int i )
{
	std::cout << "token = host, ";
	std::cout << "i = " << i << "\n";

	check_ipAdress();

	return(1);
}

int handle_server_name( std::vector<std::string> &tokens, int i )
{
	std::cout << "token = sn, ";
	std::cout << "i = " << i << "\n";
	return(1);
}

int handle_root( std::vector<std::string> &tokens, int i )
{
	std::cout << "token = root, ";
	std::cout << "i = " << i << "\n";
	return(1);
}

int handle_index( std::vector<std::string> &tokens, int i )
{
	std::cout << "token = index, ";
	std::cout << "i = " << i << "\n";
	return(1);
}

int handle_autoindex( std::vector<std::string> &tokens, int i )
{
	std::cout << "token = autoindex, ";
	std::cout << "i = " << i << "\n";
	return(1);
}

int handle_client_max_size( std::vector<std::string> &tokens, int i )
{
	std::cout << "token = cms, ";
	std::cout << "i = " << i << "\n";
	return(1);
}

int handle_allowed( std::vector<std::string> &tokens, int i )
{
	std::cout << "token = allowed, ";
	std::cout << "i = " << i << "\n";
	return(1);
}

int handle_error_page( std::vector<std::string> &tokens, int i )
{
	std::cout << "token = error, ";
	std::cout << "i = " << i << "\n";
	return(1);
}

int handle_return( std::vector<std::string> &tokens, int i )
{
	std::cout << "token = ret, ";
	std::cout << "i = " << i << "\n";
	return(1);
}

int handle_cgi( std::vector<std::string> &tokens, int i )
{
	std::cout << "token = cgi, ";
	std::cout << "i = " << i << "\n";
	return(1);
}
