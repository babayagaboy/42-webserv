/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configFile.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 17:37:39 by hgutterr          #+#    #+#             */
/*   Updated: 2026/11/15 17:52:02 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Server.hpp>
#include <ConfigDirectives.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>

bool    isSpecialChar(char c)
{
	return (c == '{' || c == '}' || c == ';');
}

int tokenizeConfigFile(char *configFilename, std::vector<std::string> &tokens)
{
	std::ifstream				configFile(configFilename);
	std::string					line;
	std::string					token;

	if (!configFile.is_open())
		return 0;

	while (getline(configFile, line))
	{
		std::stringstream ss(line);
		while (ss >> token)
		{
			std::string current = "";
			for (size_t i = 0; i < token.size(); ++i)
			{
				if (isSpecialChar(token[i]))
				{
					if (!current.empty())
						tokens.push_back(current);
					tokens.push_back(std::string(1, token[i]));
					current.clear();
					continue ;
				}
				current.push_back(token[i]);
			}
			if (!current.empty())
				tokens.push_back(current);
		}
	}

	/*std::cout << "Printing tokens:" << std::endl;

	for (size_t i = 0; i < tokens.size(); ++i) {
		std::cout << "token[" << i + 1 << "]: " << tokens[i] << std::endl;
	}*/

	return 1;
}

inline bool	isBlockKeyword(const std::string &token)
{
	return((token == "server") || (token == "location"));
}

int checkValueisKeyword(std::vector<std::pair<std::string, std::string> > &args_map, const std::string keywords[], size_t i)
{
	for (size_t f = 0; f < 11; ++f) {
		if (args_map[i].second == keywords[f] || isBlockKeyword(args_map[i].second))
		{
			std::cout << "Config error: " << args_map[i].first << "'s argument is a key word" << std::endl;
			return 0;
		}
	}

	return 1;
}

int handle_location(const std::vector<std::string> &tokens, size_t i)
{

	if (tokens[i + 1] == "{")
	{
		std::cout << "Config error: 'location' does not have an argument" << std::endl;
		return 0;
	}

	if (tokens[i + 1][0] != '/' && tokens[i + 1] != "/")
	{
		std::cout << "Config error: location's argument is not a path" << std::endl;
		return 0;
	}

	if (tokens[i + 2] != "{")
	{
		std::cout << "Config error: 'location' has more than one argument" << std::endl;
		return 0;
	}
	
	return 1;
}

bool isServerField(const std::string &token)
{
	const std::string directives[] = {
		"listen",
		"host",
		"server_name",
		"client_max_size",
		"location"
	};

	for (size_t i = 0; i < 5; ++i) {
		if (token == directives[i])
			return true;
	}
	std::cout << "Config error: '" << token << "' field is not supported in 'server'" << std::endl;
	return false;
}

bool isLocationField(const std::string &token)
{
	const std::string directives[] = {
			"root",
			"index",
			"autoindex",
			"allowed",
			"error_page",
			"return",
			"cgi",
			"client_max_size"
	};

	for (size_t i = 0; i < 8; ++i) {
		if (token == directives[i])
			return true;
	}
	std::cout << "Config error: '" << token << "' field is not supported in 'location'" << std::endl;
	return false;
}

bool hasTokens(const std::vector<std::string>& tokens, size_t current, size_t needed) {
	return current + needed < tokens.size();
}


int parse_location(std::vector<std::string> &tokens, const std::string keyWords[], size_t &i)
{
	if (!handle_location(tokens, i))
		return 0;
	
	std::string locationKeyWords[] = {"root", "index", "autoindex", "client_max_size"};

	CounterLocation fieldCounter;

	i += 3;

	int (*functions[]) ( std::vector<std::string> &tokens, size_t &i, CounterLocation &fieldCounter) = {
		&handle_root,
		&handle_index,
		&handle_autoindex,
		&handle_location_client_max_size};
	
	while (tokens[i] != "}")
	{
		if (!checkValueisKeyword(tokens[i + 1], keyWords, tokens[i]))
			return 0;
		
		if (!isLocationField(tokens[i])) 
			return 0;
	
		if (tokens[i] == "allowed") {
			if (!handle_allowed(tokens, i, fieldCounter))
				return 0;
			continue ;
		}
		if (tokens[i] == "error_page") {
			if (!handle_error_page(tokens, keyWords, i))
				return 0;
			continue ;
		}
		if (tokens[i] == "return") {
			if (!handle_return(tokens, keyWords, i, fieldCounter))
				return 0;
			continue ;
		}
		if (tokens[i] == "cgi") {
			if (!handle_cgi(tokens, keyWords, i))
				return 0;
			continue;
		}
		for (int f = 0; f < 4; ++f)
		{
			if (!checkValueisKeyword(tokens[i + 1], keyWords, tokens[i]))
				return 0;
			if (tokens[i] == locationKeyWords[f]) {
				if (!functions[f](tokens, i, fieldCounter))
					return 0;
			}
			if (tokens[i + 2] != ";") {
				std::cout << "Config error: '" << tokens[i] << "' did not end with ';'" << std::endl;
				return (0);
			}
		}
		if (tokens[i] == "location") {
			std::cout << "Config error: A 'location' block cannot be inside another 'location' block" << std::endl;
			return 0;
		}
		if (tokens[i] == "server") {
			std::cout << "Config error: A 'server' clock cannot be inside a 'location' block" << std::endl;
			return 0;
		}
		i += 3;
	}

	if (fieldCounter.autoIndexCounter > 1 || fieldCounter.indexCounter > 1 || fieldCounter.returnCounter > 1 || fieldCounter.rootCounter > 1 || fieldCounter.allowedCounter > 1 || fieldCounter.clientMaxCounter > 1) {
		std::cout << "Config error: location's block cannot hold specific duplicate fields" << std::endl;
		return 0;
	}

	++i;

	return 1;

}

int parseServer(std::vector<std::string> &tokens, const std::string keywords[], size_t &i)
{
	std::string serverKeyWords[] = {"listen", "host", "server_name", "client_max_size"};

	if (tokens[i] != "server") {
		std::cout << "Config error: First token is not 'server'" << std::endl;
		return 0;
	}
	if (!hasTokens(tokens, i, 1)) {
		std::cout << "Unexpected end of config file" << std::endl;
		return 0;
	}
	if (tokens[i + 1] != "{") {
		std::cout << "Config error: 'server' cannot have arguments" << std::endl;
		return 0;
	}

	if (!hasTokens(tokens, i, 2)) {
		std::cout << "Unexpected end of config file" << std::endl;
		return 0;
	}

	++i;++i;
	Counter fieldCounter;

	int (*functions[]) ( std::vector<std::string> &tokens, size_t i, Counter &fieldCounter ) = {
		&handle_listen, 
		&handle_host, 
		&handle_server_name,
		&handle_client_max_size };

	while (tokens[i] != "}")
	{
		if (!checkValueisKeyword(tokens[i + 1], keywords, tokens[i]))
			return 0;

		if (!isServerField(tokens[i]))
			return 0;

		for (int f = 0; f < 4; ++f) {
			if (tokens[i] == serverKeyWords[f]) {
				if (!functions[f](tokens, i, fieldCounter))
					return 0;
			}
		}
		if (!hasTokens(tokens, i, 2)) {
			std::cout << "Unexpected end of config file" << std::endl;
			return 0;
		}
		if (tokens[i + 2] != ";" && tokens[i] != "location") {
			std::cout << "Config error: '" << tokens[i] << "' did not end with ';'" << std::endl;
			return (0);
		}
		if (tokens[i] == "location") {
			if (fieldCounter.clientMaxCounter > 1 || fieldCounter.hostCounter > 1 || fieldCounter.listenCounter > 1 || fieldCounter.serverNameCounter > 1) {
				std::cout << "Config error: server's block cannot hold duplicate fields" << std::endl;
				return 0;
			}
			if (fieldCounter.listenCounter != 1 || fieldCounter.hostCounter != 1
				|| fieldCounter.serverNameCounter != 1)
			{
				std::cout << "Config error: server requires listen, host and server_name" << std::endl;
				return 0;
			}
			if (!parse_location(tokens, keywords, i))
				return 0;
			continue ;
		}
		i += 3;
	}
	++i;

	if (fieldCounter.clientMaxCounter > 1 || fieldCounter.hostCounter > 1 || fieldCounter.listenCounter > 1 || fieldCounter.serverNameCounter > 1) {
			std::cout << "Config error: server's block cannot hold duplicate fields" << std::endl;
			return 0;
	}

	return 1;
}


int parseConfigFile(std::vector<std::string> &tokens)
{
	std::string keyWords[] = {
		"listen", 
		"host", 
		"server_name",
		"client_max_size",
		"root",
		"index",
		"autoindex",
		"allowed",
		"error_page",
		"return",
		"cgi" };

	int braceBalance = 0;

	for (size_t i = 0; i < tokens.size(); ++i) {
		if (tokens[i] == "{")
			++braceBalance;
		else if (tokens[i] == "}") {
			--braceBalance;
			if (braceBalance < 0) {
				std::cout << "Config error: Unexpected '}'" << std::endl;
				return 0;
			}
		}
	}
	if (braceBalance != 0) {
		std::cout << "Config error: Unmatched braces" << std::endl;
		return 0;
	}

	size_t i = 0;

	while (i < tokens.size()) {
		
		if (tokens[i] != "server") {
			std::cout << "Config error: Expected 'server'" << std::endl;
			return 0;
		}

		if (!parseServer(tokens, keyWords, i))
			return 0;

	}
	return 1;
}

int	checkPortCompatability(const std::vector<Server> &servers)
{
	if (servers.size() == 1)
		return 1;


	for (size_t i = 0; i < servers.size(); ++i) {
		for (size_t j = i + 1; j < servers.size(); ++j) {
			if (servers[i].serversConfs.getListenPort() == servers[j].serversConfs.getListenPort()) {
				std::cout << "Config error: Different servers cannot listen to the same port" << std::endl;
				return 0;
			}
		}
	}

	return 1;
}

int fillServerConfig(char *confFileName, std::vector<Server> &server)
{
	std::vector<std::string>	tokens;

	if (!tokenizeConfigFile(confFileName, tokens))
	{
		std::cerr << "Config error: cannot open configuration file" << std::endl;
		return 0;
	}
	if (!parseConfigFile(tokens))
		return 0;

	size_t i = 0;

	while (i < tokens.size())
	{
		Server		temp;
		ServerConf	serverConf;
		while (tokens[i] != "}")
		{
			if (tokens[i] == "listen") {
				unsigned int port = static_cast<unsigned int>(std::strtod(tokens[i + 1].c_str(), NULL));
				serverConf.setListenPort(port);
			}
			else if (tokens[i] == "host")
				serverConf.setHost(tokens[i + 1]);
			else if (tokens[i] == "server_name")
				serverConf.setServerName(tokens[i + 1]);
			else if (tokens[i] == "client_max_size") {
				size_t max = static_cast<size_t>(std::strtod(tokens[i + 1].c_str(), NULL));
				serverConf.setClientMaxSize(max);
			}
			else if (tokens[i] == "location")
			{
				Location location;
				location.setPath(tokens[i + 1]);

				while (tokens[i] != "}")
				{
					if (tokens[i] == "root")
						location.setDefaultRoot(tokens[i + 1]);
					if (tokens[i] == "index")
						location.setIndex(tokens[i + 1]);
					if (tokens[i] == "autoindex")
						location.setAutoIndex(tokens[i + 1]);
					if (tokens[i] == "client_max_size")
						location.setClientMaxSize(static_cast<size_t>(std::strtod(tokens[i + 1].c_str(), NULL)));
					if (tokens[i] == "allowed" && !(tokens[i + 1].empty())) {
						++i;
						while (tokens[i] != ";") {
							location.setAllowedMethods(tokens[i]);
							++i;
						}
					}
					if (tokens[i] == "error_page") {
						int error =static_cast<int>(std::strtod(tokens[i + 1].c_str(), NULL));
						location.setErrorPage(error, tokens[i + 2]);
					}
					if (tokens[i] == "return") {
						int ret = static_cast<int>(std::strtod(tokens[i + 1].c_str(), NULL));
						if (tokens[i + 2] == ";")
							location.setReturn(ret, "");
						else
							location.setReturn(ret, tokens[i + 2]);
					}
					if (tokens[i] == "cgi")
						location.setCgi(tokens[i + 1], tokens[i + 2]);
					++i;
				}
				++i;
				serverConf.setObjLocs(location);
				continue ;
			}
			++i;
		}
		++i;
		temp.serversConfs = serverConf;
		server.push_back(temp);
	}

	if (!checkPortCompatability(server))
		return 0;
	return 1;
}

Counter::Counter() : listenCounter(0), hostCounter(0), clientMaxCounter(0), serverNameCounter(0) {}

CounterLocation::CounterLocation() : returnCounter(0), rootCounter(0), indexCounter(0), autoIndexCounter(0), allowedCounter(0), clientMaxCounter(0) {}
