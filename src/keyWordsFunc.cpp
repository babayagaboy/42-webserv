/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   keyWordsFunc.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 17:33:26 by hgutterr          #+#    #+#             */
/*   Updated: 2026/08/02 19:37:52 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Server.hpp>
#include <iostream>

int		check_ipAdress(const std::string &token)
{
	std::string line;
	double		num = 0.0;
	int			range = 0;
	int			count = 0;
	char		*end;

	std::stringstream ss(token);

	while (std::getline(ss, line, '.'))
	{
		num = std::strtod(line.c_str(), &end);
		range = static_cast<int>(num);

		std::cout << "NOW CHECKING: " << line << std::endl;
		std::cout << "NUM is: '" << num << "' and END is: '" << *end << "'" << std::endl;
		std::cout << "RANGE IS: '" << range << "'" << std::endl;
		if (range != num || *end != '\0')
		{
			std::cout << "Range is different than num" << std::endl;
			return 0;
		}
		
		if (range < 0 || range > 255 || *end != '\0')
		{
			std::cout << "Range is out of range" << std::endl;
			return 0;
		}
		
		++count;
	}

	if (count != 4)
	{
		std::cout << "Count is: " << count << std::endl;
		return 0;
	}

	return 1;
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

	if (check_ipAdress(tokens[i + 1]) == 0)
		return 0;
	
	if (tokens[i + 2] != ";")
		return 0;

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
