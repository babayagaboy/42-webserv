/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 16:16:22 by myivanov          #+#    #+#             */
/*   Updated: 2026/08/04 16:34:33 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Server.hpp>

std::string exampleSend =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "Date: Fri, 21 Jun 2024 14:18:33 GMT\r\n"
    "Last-Modified: Thu, 17 Oct 2019 07:18:26 GMT\r\n"
    "Content-Length: 1234\r\n"
    "\r\n"
    "<!DOCTYPE html>\r\n"
    "<html lang=\"en\">\r\n"
    "<head>\r\n"
    "    <meta charset=\"UTF-8\">\r\n"
    "    <title>Webserv Test</title>\r\n"
    "    <style>\r\n"
    "        body { margin:0; padding:0; background:#1e1e2f; color:white; font-family:Arial,sans-serif; display:flex; justify-content:center; align-items:center; height:100vh; }\r\n"
    "        .card { background:#2d2d44; padding:40px; border-radius:12px; text-align:center; box-shadow:0 0 20px rgba(0,0,0,0.4); }\r\n"
    "        h1 { color:#4CAF50; }\r\n"
    "        code { background:#1b1b28; padding:3px 6px; border-radius:4px; color:#ffcc66; }\r\n"
    "    </style>\r\n"
    "</head>\r\n"
    "<body>\r\n"
    "    <div class=\"card\">\r\n"
    "        <h1>🚀 Webserv is Running!</h1>\r\n"
    "        <p>If you can read this page, your HTTP server is working correctly.</p>\r\n"
    "        <p>Status: <code>HTTP/1.1 200 OK</code></p>\r\n"
    "        <hr>\r\n"
    "        <p>Made with ❤️ in C++98.</p>\r\n"
    "    </div>\r\n"
    "</body>\r\n"
    "</html>\r\n";

	
int handle_listen( std::vector<std::pair<std::string, std::string>> &args_map, size_t i );
int handle_host( std::vector<std::pair<std::string, std::string>> &args_map, size_t i );
int handle_server_name( std::vector<std::pair<std::string, std::string>> &args_map, size_t i );
int handle_root( std::vector<std::pair<std::string, std::string>> &args_map, size_t i );
int handle_index( std::vector<std::pair<std::string, std::string>> &args_map, size_t i );
int handle_autoindex( std::vector<std::pair<std::string, std::string>> &args_map, size_t i );
int handle_client_max_size( std::vector<std::pair<std::string, std::string>> &args_map, size_t i );
int handle_return( std::vector<std::pair<std::string, std::string>> &args_map, size_t i );
int handle_cgi( std::vector<std::pair<std::string, std::string>> &args_map, size_t i );

	

HTTPrequest fill_HTTP_object(std::stringstream &ss);
void    print_info(const HTTPrequest &obj);

Server::Server(int fd, sockaddr_in addr, std::vector<pollfd> &pollfds, std::map<int, Client> &clientMap) {
    serverSocket = fd;
    socketAddress = addr;
    pollfds_vector = pollfds;
    address_size = sizeof(socketAddress);
    clients = clientMap;
}

void Server::acceptNewClient() {
	int clientFd =  accept(serverSocket, (struct sockaddr *)&socketAddress, &address_size);
	if (clientFd == -1) {
		std::cout << "Failed to accept incoming connection. No valid client socket fd was created" << std::endl;
		return ;
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
    if (client.bytes_read == static_cast<size_t>(-1)) {
        std::cout << "recv() failed" << std::endl;
        return (disconnectClient(i), true);
    }

    if (client.bytes_read == 0)
        return (disconnectClient(i), true);

    client.recvBuffer.append(buff, client.bytes_read);

    if (client.recvBuffer.find("\r\n\r\n") == std::string::npos)
        return false;

    std::stringstream ss(client.recvBuffer);
    client.request = fill_HTTP_object(ss);

    if (client.request.headers.find("Content-Length") != client.request.headers.end()) {
        size_t headerEnd = client.recvBuffer.find("\r\n\r\n");
        size_t bodyStart = headerEnd + 4;

        size_t receivedBodySize = client.recvBuffer.size() - bodyStart;

        std::ostringstream oss;
        oss << receivedBodySize;

        if (client.request.headers["Content-Length"] != oss.str())
            return false;
    }
    print_info(client.request);
	
	
	send(pollfds_vector[i].fd, exampleSend.c_str(), exampleSend.size(), 0);
    
	
	std::cout << std::endl << std::endl;
    return false;
}

void Server::disconnectClient(size_t i)
{
    int clientFd = pollfds_vector[i].fd;
    close(clientFd);
    clients.erase(clientFd);
    pollfds_vector.erase(pollfds_vector.begin() + i);
}

void Server::run()
{    
	while (true)
    {
        if (poll(pollfds_vector.data(), pollfds_vector.size(), -1) == -1)
            return;

        for (size_t i = 0; i < pollfds_vector.size(); ++i)
        {
            if (pollfds_vector[i].fd == serverSocket && (pollfds_vector[i].revents & POLLIN)) {
                acceptNewClient();
            }
            else if (pollfds_vector[i].revents & POLLIN) {
                if (receiveFromClient(i)) {
                    --i;
                    continue;
        	    }
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

	std::cout << "Printing tokens:" << std::endl;

	for (size_t i = 0; i < tokens.size(); ++i) {
		std::cout << "token[" << i + 1 << "]: " << tokens[i] << std::endl;
	}

	return 0;
}

inline bool	isBlockKeyword(const std::string &token)
{
	return((token == "server") || (token == "location"));
}

int checkValueisKeyword(std::vector<std::pair<std::string, std::string>> &args_map, const std::string keywords[], size_t i)
{
    for (size_t f = 0; f < 11; ++f) {
        if (args_map[i].second == keywords[f])
        {
            std::cout << "Config error: " << args_map[i].first << "'s argument is a key word" << std::endl;
            return 0;
        }
    }

    return 1;
}

int checkValueisKeyword(const std::string &token, const std::string keywords[], size_t i)
{
    for (size_t f = 0; f < 11; ++f) {
        if (token == keywords[f])
        {
            std::cout << "Config error: " << "error_pages's argument is a key word" << std::endl;
            return 0;
        }
    }

    return 1;
}

bool    isMethod(const std::string &token)
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
            return true;
    }
    return false;
}

int handle_allowed(std::vector<std::pair<std::string, std::string>> &args_map, const std::vector<std::string> &tokens, size_t &i)
{
    int start = i;
    for (size_t s = start + 1; s < tokens.size(); ++s)
    {
        if (tokens[start + 1] == ";")
        {
            std::cout << "Config error: 'allowed' has no argument/arguments" << std::endl;
            return 0;
        }
        
        if (tokens[s] == ";")
            break;
                        
        if (!isMethod(tokens[s]))
        {
            std::cout << "Config error: Unkonwn method found in 'allowed'" << std::endl;
            return 0;
        }

        args_map.push_back(std::make_pair("allowed", tokens[s]));
        ++i;
    }
    return 1;
}

int handle_error_page(const std::vector<std::string> &tokens, const std::string keyWords[], size_t &i)
{
    int start = i;
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
            break ;

        if (!checkValueisKeyword(tokens[s], keyWords, s))
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


int parseConfigFile(std::vector<std::string> &tokens, std::vector<std::pair<std::string, std::string>> &args_map)
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
	
	int (*functions[]) ( std::vector<std::pair<std::string, std::string>> &args_map, size_t i ) = {
		&handle_listen, 
		&handle_host, 
		&handle_server_name,
        &handle_client_max_size,
		&handle_root,
		&handle_index,
		&handle_autoindex,
		&handle_return,
		&handle_cgi };

	int curlyBraces = 0;
	int blocks = 0;


	if (tokens[0] != "server")
		return (0);
	for (size_t i = 0; i < tokens.size(); ++i)
    {
		if(tokens[i] == "{") { ++curlyBraces; }
		if(tokens[i] == "}") { ++curlyBraces; }
		if(isBlockKeyword(tokens[i])) { ++blocks; }
		for (size_t f = 0; f < 11; ++f)
        {
			if(tokens[i] == keyWords[f])
            {
                if (tokens[i] == "allowed")
                {
                    if (!handle_allowed(args_map, tokens, i))
                        return 0;

                    continue ;
                }

                if (tokens[i] == "error_page")
                {
                    if (!handle_error_page(tokens, keyWords, i))
                        return 0;

                    continue ;
                }
                args_map.push_back(std::make_pair(tokens[i], tokens[i + 1]));

                if (!checkValueisKeyword(args_map, keyWords, args_map.size() - 1))
                    return 0;
                    
                if (!functions[f]( args_map, args_map.size() - 1 ))
                    return 0;
                    
                if (tokens[i + 2] != ";")
                {
                    std::cout << "Config error: " << tokens[i] << " did not end with ';'" << std::endl;
                    return (0);
                }
                
			}
		}

	}
	if(curlyBraces % 2 != 0 || blocks * 2 != curlyBraces)
	{
		return (0);
	}
	return (1);
}



int fillServerConfig(char *confFileName)
{
    std::vector<std::string>	tokens;
    std::vector<std::pair<std::string, std::string> > args_map;

	tokenizeConfigFile(confFileName, tokens);
	if(!parseConfigFile(tokens, args_map))
        return 0;

	
	return 1;
}