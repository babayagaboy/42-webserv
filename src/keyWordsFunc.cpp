/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   keyWordsFunc.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 17:33:26 by hgutterr          #+#    #+#             */
/*   Updated: 2026/08/04 16:57:21 by myivanov         ###   ########.fr       */
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

		if (range != num || *end != '\0')
			return 0;
		
		if (range < 0 || range > 255 || *end != '\0')
			return 0;
		
		++count;
	}

	if (count != 4)
		return 0;

	return 1;
}

int handle_listen(std::vector<std::pair<std::string, std::string>> &args_map, size_t i )
{
	int listenPort;
	double	num;
	char	*end;

	std::cout << "token = listen, ";
	std::cout << "i = " << i << "\n";

	if (args_map[i].second.empty() || args_map[i].second == ";")
	{
		std::cout << "Config error: 'listen' has no argument" << std::endl;
		return 0;
	}
	
	num = std::strtod((args_map[i].second).c_str(), &end);
	listenPort = static_cast<int>(num);
	if (listenPort != num || *end != '\0')
	{
		std::cout << "Config error: listen port is not a whole number" << std::endl;
		return 0;
	}
	
	if (listenPort < 1 || listenPort > 65535)
	{
		std::cout << "Config error: listen port is an invalid number" << std::endl;
		return 0;
	}
	
	return 2;
}

int handle_host( std::vector<std::pair<std::string, std::string>> &args_map, size_t i )
{
	std::cout << "token = host, ";
	std::cout << "i = " << i << "\n";

	if (args_map[i].second.empty() || args_map[i].second == ";")
	{
		std::cout << "Config error: 'host' has no argument" << std::endl;
		return 0;
	}

	if (check_ipAdress(args_map[i].second) == 0)
	{
		std::cout << "Config error: host's IP address is invalid" << std::endl;
		return 0;
	}
	
	return (1);
}

int handle_server_name( std::vector<std::pair<std::string, std::string>> &args_map, size_t i )
{
	std::cout << "token = server name, ";
	std::cout << "i = " << i << "\n";

	if (args_map[i].second.empty() || args_map[i].second == ";")
	{
		std::cout << "Config error: 'server_name' has no argument" << std::endl;
		return 0;
	}

	std::string server_name = args_map[i].second;

	for (size_t i = 0; i < server_name.size(); ++i)
	{
		if (!std::isalnum(static_cast<unsigned char>(server_name[i])) && server_name[i] != '.' && server_name[i] != '-' && server_name[i] != '_')
		{
			std::cout << "Config error: server_name contains ilegal chars" << std::endl;
			return 0;
		}
	}

	return(1);
}

int handle_client_max_size( std::vector<std::pair<std::string, std::string>> &args_map, size_t i )
{
	std::cout << "token = cms, ";
	std::cout << "i = " << i << "\n";

	int		bodySize;
	double	num;
	char	*end;

	if (args_map[i].second.empty() || args_map[i].second == ";")
	{
		std::cout << "Config error: 'client_max_size' has no argument" << std::endl;
		return 0;
	}

	num = std::strtod((args_map[i].second).c_str(), &end);
	bodySize = static_cast<int>(num);
	if (bodySize != num || *end != '\0')
	{
		std::cout << "Config error: client_max_size is not a whole number" << std::endl;
		return 0;
	}
	
	if (bodySize < 1)
	{
		std::cout << "Config error: client_max_size is not a positive number" << std::endl;
		return 0;
	}

	return(1);
}

int handle_root( std::vector<std::pair<std::string, std::string>> &args_map, size_t i )
{
	std::cout << "token = root, ";
	std::cout << "i = " << i << "\n";

	if (args_map[i].second.empty() || args_map[i].second == ";")
	{
		std::cout << "Config error: 'root' has no argument" << std::endl;
		return 0;
	}
	
	return(1);
}

int handle_index( std::vector<std::pair<std::string, std::string>> &args_map, size_t i )
{
	std::cout << "token = index, ";
	std::cout << "i = " << i << "\n";

	if (args_map[i].second.empty() || args_map[i].second == ";")
	{
		std::cout << "Config error: 'index' has no argument" << std::endl;
		return 0;
	}

	return(1);
}

int handle_autoindex( std::vector<std::pair<std::string, std::string>> &args_map, size_t i )
{
	std::cout << "token = autoindex, ";
	std::cout << "i = " << i << "\n";

	if (args_map[i].second.empty() || args_map[i].second == ";")
	{
		std::cout << "Config error: 'autoindex' has no argument" << std::endl;
		return 0;
	}

	if (args_map[i].second != "on" && args_map[i].second != "off")
	{
		std::cout << "Config error: invalid argument provided to 'autoindex'" << std::endl;
		return 0;
	}

	return(1);
}


int handle_cgi( std::vector<std::pair<std::string, std::string>> &args_map, size_t i )
{
	std::cout << "token = cgi, ";
	std::cout << "i = " << i << "\n";

	if (args_map[i].second.empty() || args_map[i].second == ";")
		return 0;

	return(1);
}


