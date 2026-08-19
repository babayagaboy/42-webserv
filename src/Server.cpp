/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 16:16:22 by myivanov          #+#    #+#             */
/*   Updated: 2026/08/19 11:50:05 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Server.hpp>
	
int handle_listen(std::vector<std::string> &tokens, size_t i, Counter &fieldCounter );
int handle_host(std::vector<std::string> &tokens, size_t i, Counter &fieldCounter );
int handle_server_name(std::vector<std::string> &tokens, size_t i, Counter &fieldCounter );
int handle_root( std::vector<std::string> &tokens, size_t &i, CounterLocation &fieldCounter);
int handle_index( std::vector<std::string> &tokens, size_t &i, CounterLocation &fieldCounter);
int handle_autoindex( std::vector<std::string> &tokens, size_t &i, CounterLocation &fieldCounter );
int handle_client_max_size( std::vector<std::string> &tokens, size_t i, Counter &fieldCounter);

int handle_cgi(const std::vector<std::string> &tokens, const std::string keyWords[], size_t &i);
int handle_return(const std::vector<std::string> &tokens, const std::string keyWords[], size_t &i, CounterLocation &fieldCounter);
int handle_error_page(const std::vector<std::string> &tokens, const std::string keyWords[], size_t &i);
int handle_allowed(const std::vector<std::string> &tokens, size_t &i, CounterLocation &fieldCounter);
bool checkExtension(const std::string &ext);
bool    isMethod(const std::string &token);
int checkValueisKeyword(const std::string &token, const std::string keywords[], const std::string &start);

void	processRequest(const Client &c, const Server &s);
	

HTTPrequest fill_HTTP_object(std::stringstream &ss);
void    print_info(const HTTPrequest &obj);

Server::Server() {}

Server::Server(int fd, sockaddr_in addr, std::vector<pollfd> &pollfds, std::map<int, Client> &clientMap) {
	serverSocket = fd;
	socketAddress = addr;
	pollfds_vector = pollfds;
	address_size = sizeof(socketAddress);
	clients = clientMap;
}

void Server::acceptNewClient()
{
	int clientFd = accept(
		serverSocket,
		(struct sockaddr *)&socketAddress,
		&address_size
	);

	if (clientFd == -1)
	{
		std::cerr << "accept() failed: "
				  << strerror(errno) << std::endl;
		return;
	}

	clients[clientFd] = Client();
	clients[clientFd].fd = clientFd;

	pollfd clientPollFd = {};
	clientPollFd.fd = clientFd;
	clientPollFd.events = POLLIN;

	pollfds_vector.push_back(clientPollFd);
}

bool Server::receiveFromClient(size_t i)
{
    int clientFd = pollfds_vector[i].fd;
    Client &client = clients[clientFd];

    char buff[4096] = {0};

    client.bytes_read = recv(client.fd, buff, sizeof(buff), 0);

	if (client.bytes_read == static_cast<size_t>(-1))
		return (disconnectClient(i), true);

	if (client.bytes_read == 0)
		return (disconnectClient(i), true);

    client.recvBuffer.append(buff, client.bytes_read);
    size_t headerEnd = client.recvBuffer.find("\r\n\r\n");

	if (headerEnd == std::string::npos)
		return false;

	std::stringstream ss(client.recvBuffer);
    client.request = fill_HTTP_object(ss);
    size_t contentLength = 0;

    std::map<std::string, std::string>::const_iterator it =
        client.request.headers.find("Content-Length");

    if (it != client.request.headers.end())
    {
        std::stringstream lengthStream(it->second);
        lengthStream >> contentLength;
    }

    size_t bodyStart = headerEnd + 4;
    size_t receivedBodySize = client.recvBuffer.size() - bodyStart;

	if (receivedBodySize < contentLength)
		return false;




	processRequest(client, *this);



	size_t requestSize = bodyStart + contentLength;

    client.recvBuffer.erase(0, requestSize);

    return false;
}

void Server::disconnectClient(size_t i)
{
	int clientFd = pollfds_vector[i].fd;
	close(clientFd);
	clients.erase(clientFd);
	pollfds_vector.erase(pollfds_vector.begin() + i);
}

int Server::getSocket() { return this->serverSocket; }



/*int	Server::verifyAllowedMethods( Client &c )
{
	if();
}*/

int Server::findLocation( const Client &c ) const
{
	const std::string &path = c.request.path;
	const std::vector<Location> &locations = serversConfs.getLocations();

	int bestLocation = -1;
	size_t bestLength = 0;

	for (size_t i = 0; i < locations.size(); ++i)
	{
		const std::string &locationPath = locations[i].getPath();

		if (path.compare(0, locationPath.size(), locationPath) != 0)
			continue;

		if (locationPath != "/" &&
			path.size() != locationPath.size() &&
			path[locationPath.size()] != '/')
			continue;

		if (locationPath.size() > bestLength)
		{
			bestLength = locationPath.size();
			bestLocation = i;
		}
	}

	return bestLocation;
}

bool Server::isMethodAllowed( const std::string &method, int l ) const
{
	const std::string *allowedMethods =
		serversConfs.getLocations()[l].getAllowedMethods();
	for (size_t i = 0; i < 9; ++i)
	{
		if (allowedMethods[i].empty())
			break;

		if (method == allowedMethods[i])
			return true;
	}
	return false;
}

void Server::run()
{
	while (true)
	{
		int ret = poll(pollfds_vector.data(), pollfds_vector.size(), -1);
		if (ret == -1) {
			std::cerr << "poll failed: " << strerror(errno) << std::endl;
			return ;
		}

		for (size_t i = 0; i < pollfds_vector.size(); ++i)
		{
			if (pollfds_vector[i].revents & POLLIN)
			{
				if (pollfds_vector[i].fd == serverSocket)
					acceptNewClient();
				else
					receiveFromClient(i);
			}
		}
	}
}


bool    isSpecialChar(char c)
{
	return (c == '{' || c == '}' || c == ';');
}

int tokenizeConfigFile(char *configFilename, std::vector<std::string> &tokens)
{
	std::ifstream				configFile(configFilename);
	std::string					line;
	std::string					token;

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

	return 0;
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
			"cgi"
	};

	for (size_t i = 0; i < 7; ++i) {
		if (token == directives[i])
			return true;
	}
	std::cout << "Config error: '" << token << "' field is not supported in 'location'" << std::endl;
	return false;
}

bool hasTokens(const std::vector<std::string>& tokens, size_t current, size_t needed) {
	return current + needed < tokens.size();
}

Counter::Counter() : listenCounter(0), hostCounter(0), clientMaxCounter(0), serverNameCounter(0) {}
CounterLocation::CounterLocation() : returnCounter(0), rootCounter(0), indexCounter(0), autoIndexCounter(0), allowedCounter(0) {}

int parse_location(std::vector<std::string> &tokens, const std::string keyWords[], size_t &i)
{
	if (!handle_location(tokens, i))
		return 0;
	
	std::string locationKeyWords[] = {"root", "index", "autoindex"};

	CounterLocation fieldCounter;

	i += 3;

	int (*functions[]) ( std::vector<std::string> &tokens, size_t &i, CounterLocation &fieldCounter) = {
		&handle_root,
		&handle_index,
		&handle_autoindex};
	
	while (tokens[i] != "}")
	{
		//std::cout << "In location, handeling token: " << tokens[i] << std::endl;
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
		for (int f = 0; f < 3; ++f)
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

	if (fieldCounter.autoIndexCounter > 1 || fieldCounter.indexCounter > 1 || fieldCounter.returnCounter > 1 || fieldCounter.rootCounter > 1 || fieldCounter.allowedCounter > 1) {
		std::cout << "Config error: location's block cannot hold specific duplicate fields" << std::endl;
		return 0;
	}

	//std::cout << "Ended location on token: " << tokens[i] << std::endl;
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

	//std::cout << "HERE IS THE TOKEN: " << tokens[i] << std::endl;

	int (*functions[]) ( std::vector<std::string> &tokens, size_t i, Counter &fieldCounter ) = {
		&handle_listen, 
		&handle_host, 
		&handle_server_name,
		&handle_client_max_size };

	while (tokens[i] != "}")
	{
		//std::cout << "In server, handeling token: " << tokens[i] << std::endl;
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
		if (!hasTokens(tokens, i, 2))
		{
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

	//std::cout << "parseServer returning i = " << i
		  //<< " token = '" << tokens[i] << "'\n";

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

	while (i < tokens.size())
	{
	   // std::cout << "Server " << i + 1 << ":" << std::endl;
		
		if (tokens[i] != "server")
		{
			std::cout << "Config error: Expected 'server'" << std::endl;
			return 0;
		}

		if (!parseServer(tokens, keyWords, i))
			return 0;

	   // std::cout << "Ended server on token: " << tokens[i] << std::endl;

	}
	return 1;
}


int fillServerConfig(char *confFileName, std::vector<Server> &server)
{
	int							serverCounter = -1;
	std::vector<std::string>	tokens;

	tokenizeConfigFile(confFileName, tokens);
	if (!parseConfigFile(tokens))
		return 0;

	size_t i = 0;

	while (i < tokens.size())
	{
		Server		temp;
		ServerConf	serverConf;
		++serverCounter;
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
					if (tokens[i] == "allowed") {
						++i;
						while (tokens[i] != ";") {
							location.setAllowedMethods(tokens[i]);
							++i;
						}
						++i;
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
	
	for (size_t i = 0; i < server.size(); ++i)
		std::cout << server[i].serversConfs << std::endl << std::endl;
	return 1;
}