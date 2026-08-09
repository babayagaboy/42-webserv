/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   keyWordsFunc.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 17:33:26 by hgutterr          #+#    #+#             */
/*   Updated: 2026/08/09 11:55:32 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Server.hpp>
#include <iostream>

bool	isBlockKeyword(const std::string &token);


int		check_ipAdress(const std::string &token)
{
	std::string line;
	double		num = 0.0;
	int			range = 0;
	int			count = 0;
	int			dotCounter = 0;
	char		*end;

	std::stringstream ss(token);

	for (size_t i = 0; i < token.size(); ++i)
	{
		if (token[i] == '.')
			++dotCounter;
	}

	if (dotCounter != 3)
		return 0;

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

int handle_listen(std::vector<std::string> &tokens, size_t i, Counter &counter)
{
	int listenPort;
	double	num;
	char	*end;

	if (tokens[i + 1].empty() || tokens[i + 1] == ";")
	{
		std::cout << "Config error: 'listen' has no argument" << std::endl;
		return 0;
	}
	
	num = std::strtod((tokens[i + 1]).c_str(), &end);
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
    ;
	++counter.listenCounter;
	return 2;
}

int handle_host( std::vector<std::string> &tokens, size_t i, Counter &counter)
{
	if (tokens[i + 1].empty() || tokens[i + 1] == ";")
	{
		std::cout << "Config error: 'host' has no argument" << std::endl;
		return 0;
	}

	if (check_ipAdress(tokens[i + 1]) == 0)
	{
		std::cout << "Config error: host's IP address is invalid" << std::endl;
		return 0;
	}
    ++counter.hostCounter;
	
	return (1);
}

int handle_server_name( std::vector<std::string> &tokens, size_t i, Counter &counter)
{
	if (tokens[i + 1].empty() || tokens[i + 1] == ";")
	{
		std::cout << "Config error: 'server_name' has no argument" << std::endl;
		return 0;
	}

	std::string server_name = tokens[i + 1];

	for (size_t i = 0; i < server_name.size(); ++i)
	{
		if (!std::isalnum(static_cast<unsigned char>(server_name[i])) && server_name[i] != '.' && server_name[i] != '-' && server_name[i] != '_')
		{
			std::cout << "Config error: server_name contains ilegal chars" << std::endl;
			return 0;
		}
	}
    ++counter.serverNameCounter;

	return(1);
}

int handle_client_max_size( std::vector<std::string> &tokens, size_t i, Counter &counter)
{
	int		bodySize;
	double	num;
	char	*end;

	if (tokens[i + 1].empty() || tokens[i + 1] == ";")
	{
		std::cout << "Config error: 'client_max_size' has no argument" << std::endl;
		return 0;
	}

	num = std::strtod((tokens[i + 1]).c_str(), &end);
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
    ++counter.clientMaxCounter;

	return(1);
}

int handle_root(std::vector<std::string> &tokens, size_t &i, CounterLocation &fieldCounter)
{
	if (tokens[i + 1].empty() || tokens[i + 1] == ";")
	{
		std::cout << "Config error: 'root' has no argument" << std::endl;
		return 0;
	}
    ++fieldCounter.rootCounter;
	
	return(1);
}

int handle_index(std::vector<std::string> &tokens, size_t &i, CounterLocation &fieldCounter )
{
	if (tokens[i + 1].empty() || tokens[i + 1] == ";")
	{
		std::cout << "Config error: 'index' has no argument" << std::endl;
		return 0;
	}

    ++fieldCounter.indexCounter;

	return(1);
}

int handle_autoindex( std::vector<std::string> &tokens, size_t &i, CounterLocation &fieldCounter)
{

	if (tokens[i + 1].empty() || tokens[i + 1] == ";")
	{
		std::cout << "Config error: 'autoindex' has no argument" << std::endl;
		return 0;
	}

	if (tokens[i + 1] != "true" && tokens[i + 1] != "false")
	{
		std::cout << "Config error: invalid argument provided to 'autoindex'" << std::endl;
		return 0;
	}
    ++fieldCounter.autoIndexCounter;

	return(1);
}

int checkValueisKeyword(const std::string &token, const std::string keywords[], const std::string &start)
{
    for (size_t f = 0; f < 11; ++f) {
        if (token == keywords[f] || isBlockKeyword(token))
        {
            std::cout << "Config error: " << start << "'s argument is a key word" << std::endl;
            return 0;
        }
    }

    return 1;
}



bool    isMethod(const std::string &token, std::vector<std::string> &foundMethods)
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
        "PATCH" };

    for (int i = 0; i < 9; ++i) {
        if (token == methods[i])
		{
			foundMethods.push_back(token);
            return true;
		}
    }
    return false;
}

int handle_allowed(const std::vector<std::string> &tokens, size_t &i, CounterLocation &fieldCounter)
{
    int start = i;

	std::vector<std::string> foundMethods;

    for (size_t s = start + 1; s < tokens.size(); ++s) {
        if (tokens[start + 1] == ";") {
            std::cout << "Config error: 'allowed' has no argument/arguments" << std::endl;
            return 0;
        }

		for (size_t k = 0; k < foundMethods.size(); ++k)
		{
			if (tokens[s] == foundMethods[k]) {
				std::cout << "Config error: Duplicate '" << tokens[s] << "' method found in 'allowed'" << std::endl;
				return 0;
			}
		}
		
        if (tokens[s] == ";")
        {
            i = s;
            ++i;
            break;
        }
            
        if (!isMethod(tokens[s], foundMethods)) {
            std::cout << "Config error: Unkonwn method found in 'allowed'" << std::endl;
            return 0;
        }
    }
    ++fieldCounter.allowedCounter;
    return 1;
}

int handle_error_page(const std::vector<std::string> &tokens, const std::string keyWords[], size_t &i)
{
    size_t start = i;
    char *end;
    double num;
    int    errNum;
    int     count = 0;

    for (size_t s = start + 1; s < tokens.size(); ++s)
    {
        if (tokens[start + 1] == ";")
        {
            std::cout << "Config error: 'error_page' has no argument/arguments" << std::endl;
            return 0;
        }

        if (tokens[s] == ";" && count == 1)
        {
            std::cout << "Config error: 'error_page' has only one argument" << std::endl;
            return 0;
        }

        if (tokens[s] == ";" && count == 2)
        {
            i = s;
            ++i;
            break ;
        }

        if (!checkValueisKeyword(tokens[s], keyWords, tokens[start]))
                    return 0;

        if (s == start + 1)
        {
            num = std::strtod(tokens[s].c_str(), &end);
            errNum = static_cast<int>(num);
            if (errNum != num || *end != '\0')
            {
                std::cout << "Config error: 1st argument of 'error_page' is not a whole number" << std::endl;
                return 0;
            }
            if (errNum < 300 || errNum > 599)
            {
                std::cout << "Config error: 1st argument of 'error_page' is not within range" << std::endl;
                return 0;
            }
        }
        ++count;
        if (count > 2)
        {
            std::cout << "Config error: 'error_page' has more than 2 arguments" << std::endl;
            return 0;
        }
        ++i;                        
    }
    return 1;
}

int handle_return(const std::vector<std::string> &tokens, const std::string keyWords[], size_t &i, CounterLocation &fieldCounter)
{
    size_t start = i;
    char *end;
    double num;
    int    errNum;
    int     count = 0;

    for (size_t s = start + 1; s < tokens.size(); ++s)
    {
        if (tokens[start + 1] == ";")
        {
            std::cout << "Config error: 'return' has no argument/arguments" << std::endl;
            return 0;
        }

        if (tokens[s] == ";" && count <= 2)
        {
            i = s;
            ++i;
            break ;
        }

        if (!checkValueisKeyword(tokens[s], keyWords, tokens[start]))
                    return 0;

        if (s == start + 1)
        {
            num = std::strtod(tokens[s].c_str(), &end);
            errNum = static_cast<int>(num);
            if (errNum != num || *end != '\0')
            {
                std::cout << "Config error: 1st argument of 'return' is not a whole number" << std::endl;
                return 0;
            }
            if (errNum < 100 || errNum > 999)
            {
                std::cout << "Config error: 1st argument of 'return' is not within range" << std::endl;
                return 0;
            }
        }
        ++count;
        if (count > 2)
        {
            std::cout << "Config error: 'return' has more than 2 arguments" << std::endl;
            return 0;
        }
        ++i;                        
    }
    ++fieldCounter.returnCounter;
    return 1;
}

bool checkExtension(const std::string &ext)
{
    if (ext.size() < 2)
        return false;

    if (ext[0] != '.')
        return false;

    return ext.find('.', 1) == std::string::npos;
}

int handle_cgi(const std::vector<std::string> &tokens, const std::string keyWords[], size_t &i)
{
    size_t start = i;
    int count = 0;

    if (tokens[start + 1] == ";")
    {
        std::cout << "Config error: 'cgi' has no arguments" << std::endl;
        return 0;
    }

    for (size_t s = start + 1; s < tokens.size(); ++s)
    {
        if (tokens[s] == ";" && count == 1)
        {
            std::cout << "Config error: 'cgi' has only one argument" << std::endl;
            return 0;
        }

        if (tokens[s] == ";" && count == 2)
        {
            i = s;
            ++i;
            break;
        }

        if (!checkValueisKeyword(tokens[s], keyWords, tokens[start]))
            return 0;

        if (count == 0)
        {
            if (!checkExtension(tokens[s]))
            {
                std::cout << "Config error: invalid CGI extension" << std::endl;
                return 0;
            }
        }

        ++count;

        if (count > 2)
        {
            std::cout << "Config error: 'cgi' has more than 2 arguments" << std::endl;
            return 0;
        }

        ++i;
    }
    return 1;

}
