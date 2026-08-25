/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 14:19:37 by hgutterr          #+#    #+#             */
/*   Updated: 2026/08/25 14:02:55 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <unistd.h>
#include <HTTPrequest.hpp>
#include <HTTPresponse.hpp>
#include <Server.hpp>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <netdb.h>


// HTTP/1.1 200 OK\r\n
// Content-Type: text/html; charset=UTF-8\r\n
// Date: Fri, 21 Jun 2024 14:18:33 GMT\r\n
// Last-Modified: Thu, 17 Oct 2019 07:18:26 GMT\r\n
// Content-Length: 1234\r\n
// \r\n

std::string	convertToUpperCase(std::string text);
std::string buildEnvVariavle(const std::string &name, const std::string &value);
std::vector<std::string> buildEnvironment(const Client &c, const Server &s, std::string execLoc);
int sendCGIResponse(Client &c, const std::string &cgiResponse);
std::string buildFilePath(const Location &location, const std::string &requestPath);
int getFilesFolder(const Client &c, HTTPresponse &response, const std::string &path);
int checkIPaddress( std::string ip );



int	method_GET(Client &c, Server &s, int l)
{
	HTTPresponse response;
	Location location = s.serversConfs.getLocations()[l];

	std::string path =
		buildFilePath(location, c.request.path);

	std::cout << "REQUEST PATH: " << c.request.path << std::endl;
	std::cout << "FILESYSTEM PATH: " << path << std::endl;

	struct stat pathStat;

	if (stat(path.c_str(), &pathStat) == -1)
	{
		perror("stat");
		return 0;
	}

	if (S_ISDIR(pathStat.st_mode))
	{
		getFilesFolder(c, response, path);
		return 1;
	}

	int fd = open(path.c_str(), O_RDONLY);

	if (fd < 0)
	{
		perror("open");
		return 0;
	}

	char buffer[4096];
	std::string body;
	ssize_t bytesRead;

	while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
		body.append(buffer, bytesRead);

	close(fd);

	std::stringstream ss;
	ss << body.size();

	std::vector<std::pair<std::string, std::string> > headers;

	headers.push_back(
		std::make_pair("Content-Length", ss.str()));

	headers.push_back(
		std::make_pair("Content-Type", "text/html"));

	response.setStatusCode(200);
	response.setBody(body);
	response.setHeaders(headers);

	std::string responseStr = response.buildResponse();

	send(c.fd, responseStr.c_str(), responseStr.size(), 0);

	return 1;
}

int	method_POST( Client &c, Server &s, int l )
{
	Location location = s.serversConfs.getLocations()[l];
	std::string path (location.getPagePath());
	std::string p (c.request.path);
	std::string postfix;

	char *argv[3];
	
	for (size_t i = 0; i < p.size(); ++i) {
		if (p[i] == '.') {
			postfix = p.substr(i);
			break;
		}
	}

	std::vector<std::pair<std::string, std::string > > cgis = location.getCgi();

	size_t j = 0;
	for (; j < cgis.size(); ++j) {
		if (postfix == cgis[j].first)
			break ;
	}
	if (j == cgis.size())
    	return -1;

	argv[0] = const_cast<char *>("/usr/bin/python3"); //to do
	argv[1] = const_cast<char *>(cgis[j].second.c_str());
	argv[2] = NULL;

	std::vector<std::string> tempEnvp = buildEnvironment(c, s, path);

	size_t i = tempEnvp.size();

	char *envp[i + 1];

	size_t k = 0;

	for (; k < tempEnvp.size(); ++k) {
		envp[k] = const_cast<char *>(tempEnvp[k].c_str());
	}
	++k;
	envp[k] = NULL;


	int	pipeToCgi[2];
	int	pipeFromCgi[2];

	if (pipe(pipeToCgi) == -1){
		std::cout << "PIPE ERROR: error while creating pipeToCgi" << std::endl;
		return -1;
	}

	if (pipe(pipeFromCgi) == -1) {
		std::cout << "PIPE ERROR: error while creating pipeFromCgi" << std::endl;
		close (pipeToCgi[0]);
		close (pipeToCgi[1]);
		return -1;
	}

	pid_t pid = fork();
	if (pid == -1) {
		std::cout << "FORK ERROR: error while creating child process" << std::endl;
		close (pipeToCgi[0]);
		close (pipeToCgi[1]);
		close(pipeFromCgi[0]);
		close(pipeFromCgi[1]);
		return -1;
	}

	if (pid == 0) {
		close(pipeToCgi[1]);
		close(pipeFromCgi[0]);

		if (dup2(pipeToCgi[0], STDIN_FILENO) == -1) {
			std::cout << "Error while duplicating / redirecting pipeToCgi[0]" << std::endl;
			return -1;
		}

		if (dup2(pipeFromCgi[1], STDOUT_FILENO) == -1) {
			std::cout << "Error while duplicating / redirecring pipeFromCgi[1]" << std::endl;
			return -1;
		}

		close(pipeToCgi[0]);
		close(pipeFromCgi[1]);

		execve(argv[0], argv, envp);
		exit(0);
	}
	else
	{
		close(pipeToCgi[0]);
		close(pipeFromCgi[1]);

		c.cgiInputFd = pipeToCgi[1];
		c.cgiOutputFd = pipeFromCgi[0];

		c.cgiBody = c.request.body;
		c.cgiBodyOffset = 0;

		c.cgiResponse.clear();

		pollfd stdinCgi;
		stdinCgi.fd = c.cgiInputFd;
		stdinCgi.events = POLLOUT;
		stdinCgi.revents = 0;

		pollfd stdoutCgi;
		stdoutCgi.fd = c.cgiOutputFd;
		stdoutCgi.events = POLLIN;
		stdoutCgi.revents = 0;

		s.pollfds_vector.push_back(stdinCgi);
		s.pollfds_vector.push_back(stdoutCgi);
	}
	
	return 1;
}



/*if (write(pipeToCgi[1], body.c_str(), body.size()) == -1) {
			std::cout << "Error writing body to CGI" << std::endl;
		}
		close(pipeToCgi[1]);

		char buffer[4096];
		std::string cgiResponse;
		ssize_t bytesRead;
		
		while ((bytesRead = read(pipeFromCgi[0], buffer, sizeof(buffer))) > 0)
			cgiResponse.append(buffer, bytesRead);

		close(pipeFromCgi[0]);
		waitpid(pid, NULL, 0);

		sendCGIResponse(c, cgiResponse);*/




int	method_DELETE(Client &c, Server &s, int l)
{
		Location location = s.serversConfs.getLocations()[l];
	std::string path (location.getPagePath());
	std::string p (c.request.path);
	std::string postfix;

	char *argv[3];
	
	for (size_t i = 0; i < p.size(); ++i) {
		if (p[i] == '.') {
			postfix = p.substr(i);
			break;
		}
	}

	std::vector<std::pair<std::string, std::string > > cgis = location.getCgi();

	size_t j = 0;
	for (; j < cgis.size(); ++j) {
		if (postfix == cgis[j].first)
			break ;
	}
	if (j == cgis.size())
    	return -1;

	argv[0] = const_cast<char *>("/usr/bin/python3"); //to do
	argv[1] = const_cast<char *>(cgis[j].second.c_str());
	argv[2] = NULL;

	std::vector<std::string> tempEnvp = buildEnvironment(c, s, path);

	size_t i = tempEnvp.size();

	char *envp[i + 1];

	size_t k = 0;

	for (; k < tempEnvp.size(); ++k) {
		envp[k] = const_cast<char *>(tempEnvp[k].c_str());
	}
	++k;
	envp[k] = NULL;


	int	pipeToCgi[2];
	int	pipeFromCgi[2];

	if (pipe(pipeToCgi) == -1){
		std::cout << "PIPE ERROR: error while creating pipeToCgi" << std::endl;
		return -1;
	}

	if (pipe(pipeFromCgi) == -1) {
		std::cout << "PIPE ERROR: error while creating pipeFromCgi" << std::endl;
		close (pipeToCgi[0]);
		close (pipeToCgi[1]);
		return -1;
	}

	pid_t pid = fork();
	if (pid == -1) {
		std::cout << "FORK ERROR: error while creating child process" << std::endl;
		close (pipeToCgi[0]);
		close (pipeToCgi[1]);
		close(pipeFromCgi[0]);
		close(pipeFromCgi[1]);
		return -1;
	}

	if (pid == 0) {
		close(pipeToCgi[1]);
		close(pipeFromCgi[0]);

		if (dup2(pipeToCgi[0], STDIN_FILENO) == -1) {
			std::cout << "Error while duplicating / redirecting pipeToCgi[0]" << std::endl;
			return -1;
		}

		if (dup2(pipeFromCgi[1], STDOUT_FILENO) == -1) {
			std::cout << "Error while duplicating / redirecring pipeFromCgi[1]" << std::endl;
			return -1;
		}

		close(pipeToCgi[0]);
		close(pipeFromCgi[1]);

		execve(argv[0], argv, envp);
		exit(0);
	}
	else
	{
		close(pipeToCgi[0]);
		close(pipeFromCgi[1]);

		c.cgiInputFd = pipeToCgi[1];
		c.cgiOutputFd = pipeFromCgi[0];

		c.cgiBody = c.request.body;
		c.cgiBodyOffset = 0;

		c.cgiResponse.clear();

		pollfd stdinCgi;
		stdinCgi.fd = c.cgiInputFd;
		stdinCgi.events = POLLOUT;
		stdinCgi.revents = 0;

		pollfd stdoutCgi;
		stdoutCgi.fd = c.cgiOutputFd;
		stdoutCgi.events = POLLIN;
		stdoutCgi.revents = 0;

		s.pollfds_vector.push_back(stdinCgi);
		s.pollfds_vector.push_back(stdoutCgi);
	}
	
	return 1;
}

int	method_PUT( Client &c, Server &s, int l )
{
		Location location = s.serversConfs.getLocations()[l];
	std::string path (location.getPagePath());
	std::string p (c.request.path);
	std::string postfix;

	char *argv[3];
	
	for (size_t i = 0; i < p.size(); ++i) {
		if (p[i] == '.') {
			postfix = p.substr(i);
			break;
		}
	}

	std::vector<std::pair<std::string, std::string > > cgis = location.getCgi();

	size_t j = 0;
	for (; j < cgis.size(); ++j) {
		if (postfix == cgis[j].first)
			break ;
	}
	if (j == cgis.size())
    	return -1;

	argv[0] = const_cast<char *>("/usr/bin/python3"); //to do
	argv[1] = const_cast<char *>(cgis[j].second.c_str());
	argv[2] = NULL;

	std::vector<std::string> tempEnvp = buildEnvironment(c, s, path);

	size_t i = tempEnvp.size();

	char *envp[i + 1];

	size_t k = 0;

	for (; k < tempEnvp.size(); ++k) {
		envp[k] = const_cast<char *>(tempEnvp[k].c_str());
	}
	++k;
	envp[k] = NULL;


	int	pipeToCgi[2];
	int	pipeFromCgi[2];

	if (pipe(pipeToCgi) == -1){
		std::cout << "PIPE ERROR: error while creating pipeToCgi" << std::endl;
		return -1;
	}

	if (pipe(pipeFromCgi) == -1) {
		std::cout << "PIPE ERROR: error while creating pipeFromCgi" << std::endl;
		close (pipeToCgi[0]);
		close (pipeToCgi[1]);
		return -1;
	}

	pid_t pid = fork();
	if (pid == -1) {
		std::cout << "FORK ERROR: error while creating child process" << std::endl;
		close (pipeToCgi[0]);
		close (pipeToCgi[1]);
		close(pipeFromCgi[0]);
		close(pipeFromCgi[1]);
		return -1;
	}

	if (pid == 0) {
		close(pipeToCgi[1]);
		close(pipeFromCgi[0]);

		if (dup2(pipeToCgi[0], STDIN_FILENO) == -1) {
			std::cout << "Error while duplicating / redirecting pipeToCgi[0]" << std::endl;
			return -1;
		}

		if (dup2(pipeFromCgi[1], STDOUT_FILENO) == -1) {
			std::cout << "Error while duplicating / redirecring pipeFromCgi[1]" << std::endl;
			return -1;
		}

		close(pipeToCgi[0]);
		close(pipeFromCgi[1]);

		execve(argv[0], argv, envp);
		exit(0);
	}
	else
	{
		close(pipeToCgi[0]);
		close(pipeFromCgi[1]);

		c.cgiInputFd = pipeToCgi[1];
		c.cgiOutputFd = pipeFromCgi[0];

		c.cgiBody = c.request.body;
		c.cgiBodyOffset = 0;

		c.cgiResponse.clear();

		pollfd stdinCgi;
		stdinCgi.fd = c.cgiInputFd;
		stdinCgi.events = POLLOUT;
		stdinCgi.revents = 0;

		pollfd stdoutCgi;
		stdoutCgi.fd = c.cgiOutputFd;
		stdoutCgi.events = POLLIN;
		stdoutCgi.revents = 0;

		s.pollfds_vector.push_back(stdinCgi);
		s.pollfds_vector.push_back(stdoutCgi);
	}
	
	return 1;
}

int	method_HEAD( Client &c, Server &s, int l )
{
	HTTPresponse response;
	Location location = s.serversConfs.getLocations()[l];
	std::string path(location.getPagePath());
	
	struct stat fileInfo;


    if (stat(path.c_str(), &fileInfo) == -1)
	{
        return 0;
	}

    std::stringstream ss;
    ss << fileInfo.st_size;

	std::vector<std::pair<std::string, std::string> > headers;

	headers.push_back(std::make_pair("Content-Length", ss.str()));
	headers.push_back(std::make_pair("Content-Type", "text/html"));
	
	response.setStatusCode(200);
	response.setHeaders(headers);
	
	std::string responseStr = response.buildResponse();

	
	std::cout << "\n\nraw response : " << responseStr << std::endl;
	send(c.fd, responseStr.c_str(), responseStr.size(), 0);
	return 1;
}

int	method_OPTIONS( Client &c, Server &s, int l )
{
	HTTPresponse response;
	Location location = s.serversConfs.getLocations()[l];
	const std::string *allowedMethods = location.getAllowedMethods();

	std::string allow;

	for (size_t i = 0; i < 9; ++i) {
		if (allowedMethods[i].empty())
			break;
		if (!allow.empty())
			allow += ", ";

		allow += allowedMethods[i];
	}

	std::vector<std::pair<std::string, std::string> > headers;

	headers.push_back(std::make_pair("Allow", allow));
	headers.push_back(std::make_pair("Content-Length", "0"));

	response.setStatusCode(204);
	response.setHeaders(headers);

	std::string responseStr = response.buildResponse();

	send(c.fd, responseStr.c_str(), responseStr.size(), 0);
	return 1;
}

int	method_TRACE( Client &c, Server &s, int l )
{

	std::cout << "HI I AM IN 'TRACE'" << std::endl;
	static_cast<void>(s);
	static_cast<void>(l);

	HTTPresponse response;
	std::string resBody;
	std::map<std::string, std::string>::const_iterator it;

	resBody = c.request.method + " " + c.request.path + " " + c.request.version + "\r\n";
	
	for (it = c.request.headers.begin(); it != c.request.headers.end(); ++it) {
		resBody += it->first + ": " + it->second + "\r\n";
	}

	resBody += "\r\n" + c.request.body;

	std::vector<std::pair<std::string, std::string> > headers;
	std::stringstream ss;

	ss << resBody.size();

	std::string resBodySize = ss.str();

	headers.push_back(std::make_pair("Content-Type", "message/http"));
	headers.push_back(std::make_pair("Content-Length", resBodySize));

	response.setStatusCode(200);
	response.setHeaders(headers);
	response.setBody(resBody);

	std::string responseStr = response.buildResponse();

	send(c.fd, responseStr.c_str(), responseStr.size(), 0);
	return 1;
}


int	method_PATCH( Client &c, Server &s, int l )
{
	Location location = s.serversConfs.getLocations()[l];
	std::string path (location.getPagePath());
	std::string p (c.request.path);
	std::string postfix;

	char *argv[3];
	
	for (size_t i = 0; i < p.size(); ++i) {
		if (p[i] == '.') {
			postfix = p.substr(i);
			break;
		}
	}

	std::vector<std::pair<std::string, std::string > > cgis = location.getCgi();

	size_t j = 0;
	for (; j < cgis.size(); ++j) {
		if (postfix == cgis[j].first)
			break ;
	}
	if (j == cgis.size())
    	return -1;

	//argv[0] = const_cast<char *>(cgis[j].second.c_str());
	argv[0] = const_cast<char *>("/usr/bin/python3"); //to do
	argv[1] = const_cast<char *>(cgis[j].second.c_str());
	argv[2] = NULL;

	std::vector<std::string> tempEnvp = buildEnvironment(c, s, path);

	size_t i = tempEnvp.size();

	char *envp[i + 1];

	size_t k = 0;

	for (; k < tempEnvp.size(); ++k) {
		envp[k] = const_cast<char *>(tempEnvp[k].c_str());
	}
	++k;
	envp[k] = NULL;


	int	pipeToCgi[2];
	int	pipeFromCgi[2];

	if (pipe(pipeToCgi) == -1){
		std::cout << "PIPE ERROR: error while creating pipeToCgi" << std::endl;
		return -1;
	}

	if (pipe(pipeFromCgi) == -1) {
		std::cout << "PIPE ERROR: error while creating pipeFromCgi" << std::endl;
		return -1;
	}

	pid_t pid = fork();
	if (pid == -1) {
		std::cout << "FORK ERROR: error while creating child process" << std::endl;
		return -1;
	}

	if (pid == 0) {
		close(pipeToCgi[1]);
		close(pipeFromCgi[0]);

		if (dup2(pipeToCgi[0], STDIN_FILENO) == -1) {
			std::cout << "Error while duplicating / redirecting pipeToCgi[0]" << std::endl;
			return -1;
		}

		if (dup2(pipeFromCgi[1], STDOUT_FILENO) == -1) {
			std::cout << "Error while duplicating / redirecring pipeFromCgi[1]" << std::endl;
			return -1;
		}
		close(pipeToCgi[0]);
		close(pipeFromCgi[1]);

		execve(argv[0], argv, envp);
		exit(0);
	}
	else
	{
		close(pipeFromCgi[1]);
		close(pipeToCgi[0]);

		const std::string &body = c.request.body;

		if (write(pipeToCgi[1], body.c_str(), body.size()) == -1) {
			std::cout << "Error writing body to CGI" << std::endl;
		}
		close(pipeToCgi[1]);

		char buffer[4096];
		std::string cgiResponse;
		ssize_t bytesRead;
		
		while ((bytesRead = read(pipeFromCgi[0], buffer, sizeof(buffer))) > 0)
			cgiResponse.append(buffer, bytesRead);

		close(pipeFromCgi[0]);
		waitpid(pid, NULL, 0);

		send(c.fd, cgiResponse.c_str(), cgiResponse.size(), 0);
	}
	return 1;
}

int connectUpstream(const std::string &host, const std::string &port)
{
    struct addrinfo hints;
    struct addrinfo *result;
    struct addrinfo *rp;

    std::memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(
        host.c_str(),
        port.c_str(),
        &hints,
        &result
    );

    if (ret != 0)
    {
        std::cerr << "getaddrinfo: "
                  << gai_strerror(ret)
                  << "\n";
        return -1;
    }

    int fd = -1;

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        fd = socket(
            rp->ai_family,
            rp->ai_socktype,
            rp->ai_protocol
        );

        if (fd == -1)
            continue;

        if (connect(fd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(fd);
        fd = -1;
    }

    freeaddrinfo(result);

    return fd;
}

int method_CONNECT(Client &c, Server &s, int l)
{
    (void)s;
    (void)l;

    std::cout << "ENTERED CONNECT\n";

    std::string target(c.request.path);

    std::cout << "CONNECT received\n";
    std::cout << "Target: " << target << "\n";

    std::size_t colon = target.rfind(':');

    if (colon == std::string::npos)
    {
        std::cerr << "CONNECT: invalid target\n";
        return 400;
    }

    std::string host = target.substr(0, colon);
    std::string port = target.substr(colon + 1);

    if (host.empty() || port.empty())
    {
        std::cerr << "CONNECT: invalid target\n";
        return 400;
    }

    std::cout << "Host: " << host << "\n";
    std::cout << "Port: " << port << "\n";

    int upstreamFd = connectUpstream(host, port);

    if (upstreamFd == -1)
    {
        std::cerr << "CONNECT: failed to connect to upstream\n";

        std::string response =
            "HTTP/1.1 502 Bad Gateway\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

        send(c.fd, response.c_str(), response.size(), 0);

        return 502;
    }

    std::cout << "UPSTREAM CONNECTED\n";
    std::cout << "Client FD: " << c.fd << "\n";
    std::cout << "Upstream FD: " << upstreamFd << "\n";

    std::string response =
        "HTTP/1.1 200 Connection Established\r\n"
        "\r\n";

    ssize_t sent = send(
        c.fd,
        response.c_str(),
        response.size(),
        0
    );

    if (sent < 0)
    {
        perror("send CONNECT response");
        close(upstreamFd);
        return 500;
    }

    c.upstreamfd = upstreamFd;
    c.tunnel = true;

    return 0;
}

void	processRequest(Client &c, Server &s)
{
	int location = s.findLocation(c);

	/*std::cout << "server id: " << s.getServerId() << std::endl;
	std::cout << "location id: " << c.request.path << std::endl;*/

	if (location < 0 && c.request.method != "CONNECT")
	{
		std::cout << "No matching location" << std::endl;
		return;
	}
	if (c.request.method == "CONNECT")
	{
		method_CONNECT(const_cast<Client &>(c), s, location);
		return ;
	}
	
	// std::cout << "BEFORE CONNECT2\n";

	if (!s.isMethodAllowed(c.request.method, location))
	{
		std::cout	<< "Method "
				  	<< c.request.method
				  	<< " is not allowed for location "
				  	<< s.serversConfs.getLocations()[location].getPath()
				  	<< "?:"
				  	<< location
				  	<< "in server["
					<< s.getServerId()
					<< "]"
				  	<< std::endl;
		return ;
	}

	// std::cout << "BEFORE CONNECT3\n";

	std::string methods[] = {
		"GET",
		"POST",
		"DELETE",
		"PUT",
		"HEAD",
		"OPTIONS",
		"TRACE",
		"CONNECT",
		"PATCH",
		"CONNECT"
	};
	
	int (*methfunctions[]) (Client &c, Server &s, int location ) = {
		&method_GET, 
		&method_POST, 
		&method_DELETE,
        &method_PUT,
		&method_HEAD,
		&method_OPTIONS,
		&method_TRACE,
		&method_PATCH,
	};

	for (size_t i = 0; i < 9; ++i)
	{
		if(c.request.method == methods[i])
			methfunctions[i](c, s, location);
	}
	
	//print_info(c.request);
}