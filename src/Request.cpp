/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 14:19:37 by hgutterr          #+#    #+#             */
/*   Updated: 2026/09/13 16:35:52 by hgutterr         ###   ########.fr       */
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
#include <limits.h>


std::string	convertToUpperCase(std::string text);
std::string buildEnvVariavle(const std::string &name, const std::string &value);
std::vector<std::string> buildEnvironment(const Client &c, const Server &s, std::string execLoc);
int sendCGIResponse(Client &c, const std::string &cgiResponse);
std::string buildFilePath(const Location &location, const std::string &requestPath);
int getFilesFolder(Client &c, Server &s, HTTPresponse &response, const std::string &path);
int checkIPaddress( std::string ip );
std::string findCGIcompiler(const std::string& extension);
void    print_info(const HTTPrequest &obj);

static int sendConnectText(Client &c, const std::string &body, int status)
{
	std::stringstream length;
	length << body.size();
	std::stringstream response;
	response << "HTTP/1.1 " << status << (status == 200 ? " OK" : " Bad Request") << "\r\n"
		<< "Content-Type: text/plain\r\nContent-Length: " << length.str()
		<< "\r\nConnection: keep-alive\r\n\r\n" << body;
	return send(c.fd, response.str().c_str(), response.str().size(), 0);
}

static void method_CONNECTMessage(Client &c, Server &s)
{
	if (s.connectTerminalFd == -1 || s.clients.find(s.connectTerminalFd) == s.clients.end())
	{
		sendConnectText(c, "No nc client is connected\n", 400);
		return;
	}
	Client &terminal = s.clients[s.connectTerminalFd];
	if (send(terminal.fd, c.request.body.c_str(), c.request.body.size(), 0) < 0)
		sendConnectText(c, "Could not write to nc client\n", 400);
	else
		sendConnectText(c, "sent\n", 200);
}

static void method_CONNECTStatus(Client &c, Server &s)
{
	std::string message = s.connectMessages;
	s.connectMessages.clear();
	sendConnectText(c, message, 200);
}


/*int	method_GET(Client &c, Server &s, int l)
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

	if (c.newSession)
	{
		size_t pos = responseStr.find("\r\n");
		if (pos != std::string::npos)
		{
			std::string cookieLine = std::string("\r\nSet-Cookie: SessionId=") + c.sessionId + std::string("; Path=/");
			responseStr.insert(pos + 2, cookieLine);
		}
		else
		{
			std::string cookieHeader = std::string("Set-Cookie: SessionId=") + c.sessionId + std::string("; Path=/\r\n");
			responseStr = cookieHeader + responseStr;
		}

		const_cast<Client&>(c).newSession = false;
	}

	send(c.fd, responseStr.c_str(), responseStr.size(), 0);

	return 1;
}*/


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
		s.handleError(c, l, 404);
		return 1;
	}

	if (S_ISDIR(pathStat.st_mode))
	{
		std::string indexFile = path;
		if (!indexFile.empty() && indexFile[indexFile.size() - 1] != '/')
			indexFile += '/';
		indexFile += location.getIndex();

		struct stat indexStat;
		if (!location.getIndex().empty()
			&& stat(indexFile.c_str(), &indexStat) == 0
			&& !S_ISDIR(indexStat.st_mode))
		{
			c.request.path = "/" + location.getIndex();
			std::string indexPath = buildFilePath(location, c.request.path);
			return method_GET(c, s, l);
		}

		if (location.getAutoIndex())
		{
			getFilesFolder(c, s, response, path);
			return 1;
		}

		s.handleError(c, l, 403);
		return 1;
	}

	int fd = open(path.c_str(), O_RDONLY);

	if (fd < 0)
	{
		perror("open");

		if (errno == EACCES)
			s.handleError(c, l, 403);
		else
			s.handleError(c, l, 500);

		return 1;
	}

	char buffer[4096];
	std::string body;
	ssize_t bytesRead;

	while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
		body.append(buffer, bytesRead);

	close(fd);

	if (bytesRead < 0)
	{
		perror("read");
		s.handleError(c, l, 500);
		return 1;
	}

	std::stringstream ss;
	ss << body.size();

	std::vector<std::pair<std::string, std::string> > headers;

	headers.push_back(
		std::make_pair("Content-Length", ss.str())
	);

	headers.push_back(
		std::make_pair("Content-Type", "text/html")
	);

	response.setStatusCode(200);
	response.setBody(body);
	response.setHeaders(headers);

	std::string responseStr = response.buildResponse();

	if (c.newSession)
	{
		size_t pos = responseStr.find("\r\n");

		if (pos != std::string::npos)
		{
			std::string cookieLine =
				std::string("Set-Cookie: SessionId=")
				+ c.sessionId
				+ std::string("; Path=/\r\n");

			responseStr.insert(pos + 2, cookieLine);
		}

		c.newSession = false;
	}

	c.sendBuffer = responseStr;
	c.sendOffset = 0;

	s.enableClientWrite(c.fd);

	return 1;
}


int	method_POST(Client &c, Server &s, int l)
{
	Location location = s.serversConfs.getLocations()[l];
	std::string p = c.request.path;
	std::string postfix;

	for (size_t i = 0; i < p.size(); ++i)
	{
		if (p[i] == '.')
		{
			postfix = p.substr(i);
			break;
		}
	}

	std::vector<std::pair<std::string, std::string> > cgis =
		location.getCgi();

	size_t j = 0;

	for (; j < cgis.size(); ++j)
	{
		if (postfix == cgis[j].first)
			break;
	}

	if (j == cgis.size())
	{
		s.handleError(c, l, 500);
		return 1;
	}

	std::string compiler = findCGIcompiler(cgis[j].first);
	std::string script = cgis[j].second;

	char *argv[3];

	argv[0] = const_cast<char *>(compiler.c_str());
	argv[1] = const_cast<char *>(script.c_str());
	argv[2] = NULL;

	std::vector<std::string> tempEnvp =
		buildEnvironment(c, s, script);

	std::vector<char *> envp;

	for (size_t k = 0; k < tempEnvp.size(); ++k)
		envp.push_back(const_cast<char *>(tempEnvp[k].c_str()));

	envp.push_back(NULL);

	int pipeToCgi[2];
	int pipeFromCgi[2];

	if (pipe(pipeToCgi) == -1)
	{
		perror("pipeToCgi");
		s.handleError(c, l, 500);
		return 1;
	}

	if (pipe(pipeFromCgi) == -1)
	{
		perror("pipeFromCgi");

		close(pipeToCgi[0]);
		close(pipeToCgi[1]);

		s.handleError(c, l, 500);
		return 1;
	}

	pid_t pid = fork();

	if (pid == -1)
	{
		perror("fork");

		close(pipeToCgi[0]);
		close(pipeToCgi[1]);
		close(pipeFromCgi[0]);
		close(pipeFromCgi[1]);

		s.handleError(c, l, 500);
		return 1;
	}

	if (pid == 0)
	{
		close(pipeToCgi[1]);
		close(pipeFromCgi[0]);

		if (dup2(pipeToCgi[0], STDIN_FILENO) == -1)
			_exit(1);

		if (dup2(pipeFromCgi[1], STDOUT_FILENO) == -1)
			_exit(1);

		close(pipeToCgi[0]);
		close(pipeFromCgi[1]);

		execve(argv[0], argv, envp.data());

		perror("execve");
		_exit(127);
	}

	close(pipeToCgi[0]);
	close(pipeFromCgi[1]);

	c.cgiInputFd = pipeToCgi[1];
	c.cgiOutputFd = pipeFromCgi[0];

	std::cerr << "CGI BODY SIZE = "
			  << c.request.body.size()
			  << std::endl;

	c.cgiBody = c.request.body;
	c.cgiBodyOffset = 0;
	c.cgiResponse.clear();
	c.cgiPid = pid;

	if (fcntl(c.cgiInputFd, F_SETFL, O_NONBLOCK) == -1)
	{
		perror("fcntl CGI input");

		close(c.cgiInputFd);
		close(c.cgiOutputFd);

		c.cgiInputFd = -1;
		c.cgiOutputFd = -1;

		s.handleError(c, l, 500);
		return 1;
	}

	if (fcntl(c.cgiOutputFd, F_SETFL, O_NONBLOCK) == -1)
	{
		perror("fcntl CGI output");

		close(c.cgiInputFd);
		close(c.cgiOutputFd);

		c.cgiInputFd = -1;
		c.cgiOutputFd = -1;

		s.handleError(c, l, 500);
		return 1;
	}

	pollfd stdinCgi;

	stdinCgi.fd = c.cgiInputFd;
	stdinCgi.events = POLLOUT;
	stdinCgi.revents = 0;

	s.pollfds_vector.push_back(stdinCgi);

	pollfd stdoutCgi;

	stdoutCgi.fd = c.cgiOutputFd;
	stdoutCgi.events = POLLIN;
	stdoutCgi.revents = 0;

	s.pollfds_vector.push_back(stdoutCgi);

	return 1;
}


int	method_DELETE(Client &c, Server &s, int l)
{
	Location location = s.serversConfs.getLocations()[l];

	std::string p = c.request.path;
	std::string postfix;

	for (size_t i = 0; i < p.size(); ++i)
	{
		if (p[i] == '.')
		{
			postfix = p.substr(i);
			break;
		}
	}

	std::vector<std::pair<std::string, std::string> > cgis =
		location.getCgi();

	size_t j = 0;

	for (; j < cgis.size(); ++j)
	{
		if (postfix == cgis[j].first)
			break;
	}

	if (j == cgis.size())
	{
		s.handleError(c, l, 500);
		return 1;
	}

	std::string cgiScript = cgis[j].second;
	std::string compiler = findCGIcompiler(cgis[j].first);

	char *argv[3];

	argv[0] = const_cast<char *>(compiler.c_str());
	argv[1] = const_cast<char *>(cgiScript.c_str());
	argv[2] = NULL;

	std::vector<std::string> tempEnvp =
		buildEnvironment(c, s, cgiScript);

	std::vector<char *> envp;

	for (size_t k = 0; k < tempEnvp.size(); ++k)
		envp.push_back(
			const_cast<char *>(tempEnvp[k].c_str())
		);

	envp.push_back(NULL);

	int pipeToCgi[2];
	int pipeFromCgi[2];

	if (pipe(pipeToCgi) == -1)
	{
		std::cerr << "pipeToCgi failed: "
				  << strerror(errno) << std::endl;

		s.handleError(c, l, 500);
		return 1;
	}

	if (pipe(pipeFromCgi) == -1)
	{
		std::cerr << "pipeFromCgi failed: "
				  << strerror(errno) << std::endl;

		close(pipeToCgi[0]);
		close(pipeToCgi[1]);

		s.handleError(c, l, 500);
		return 1;
	}

	pid_t pid = fork();

	if (pid == -1)
	{
		std::cerr << "fork failed: "
				  << strerror(errno) << std::endl;

		close(pipeToCgi[0]);
		close(pipeToCgi[1]);
		close(pipeFromCgi[0]);
		close(pipeFromCgi[1]);

		s.handleError(c, l, 500);
		return 1;
	}

	if (pid == 0)
	{
		close(pipeToCgi[1]);
		close(pipeFromCgi[0]);

		if (dup2(pipeToCgi[0], STDIN_FILENO) == -1)
			_exit(1);

		if (dup2(pipeFromCgi[1], STDOUT_FILENO) == -1)
			_exit(1);

		close(pipeToCgi[0]);
		close(pipeFromCgi[1]);

		execve(argv[0], argv, envp.data());

		_exit(127);
	}

	close(pipeToCgi[0]);
	close(pipeFromCgi[1]);

	c.cgiInputFd = pipeToCgi[1];
	c.cgiOutputFd = pipeFromCgi[0];

	c.cgiBody = c.request.body;
	c.cgiBodyOffset = 0;

	c.cgiResponse.clear();
	c.cgiPid = pid;

	if (fcntl(c.cgiInputFd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "fcntl CGI input failed: "
				  << strerror(errno) << std::endl;

		close(c.cgiInputFd);
		close(c.cgiOutputFd);

		c.cgiInputFd = -1;
		c.cgiOutputFd = -1;

		s.handleError(c, l, 500);
		return 1;
	}

	if (fcntl(c.cgiOutputFd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "fcntl CGI output failed: "
				  << strerror(errno) << std::endl;

		close(c.cgiInputFd);
		close(c.cgiOutputFd);

		c.cgiInputFd = -1;
		c.cgiOutputFd = -1;

		s.handleError(c, l, 500);
		return 1;
	}

	pollfd stdinCgi;

	stdinCgi.fd = c.cgiInputFd;
	stdinCgi.events = POLLOUT;
	stdinCgi.revents = 0;

	s.pollfds_vector.push_back(stdinCgi);

	pollfd stdoutCgi;

	stdoutCgi.fd = c.cgiOutputFd;
	stdoutCgi.events = POLLIN;
	stdoutCgi.revents = 0;

	s.pollfds_vector.push_back(stdoutCgi);

	return 1;
}


int	method_PUT(Client &c, Server &s, int l)
{
	Location location = s.serversConfs.getLocations()[l];

	std::string p = c.request.path;
	std::string postfix;

	for (size_t i = 0; i < p.size(); ++i)
	{
		if (p[i] == '.')
		{
			postfix = p.substr(i);
			break;
		}
	}

	std::vector<std::pair<std::string, std::string> > cgis =
		location.getCgi();

	size_t j = 0;

	for (; j < cgis.size(); ++j)
	{
		if (postfix == cgis[j].first)
			break;
	}

	/*
	 * No CGI configured for this file extension.
	 */
	if (j == cgis.size())
	{
		s.handleError(c, l, 500);
		return 1;
	}

	std::string compiler = findCGIcompiler(cgis[j].first);
	std::string cgiScript = cgis[j].second;

	char *argv[3];

	argv[0] = const_cast<char *>(compiler.c_str());
	argv[1] = const_cast<char *>(cgiScript.c_str());
	argv[2] = NULL;

	std::vector<std::string> tempEnvp =
		buildEnvironment(c, s, cgiScript);

	std::vector<char *> envp;

	for (size_t k = 0; k < tempEnvp.size(); ++k)
	{
		envp.push_back(
			const_cast<char *>(tempEnvp[k].c_str())
		);
	}

	envp.push_back(NULL);

	int pipeToCgi[2];
	int pipeFromCgi[2];

	if (pipe(pipeToCgi) == -1)
	{
		std::cerr << "pipeToCgi failed: "
				  << strerror(errno) << std::endl;

		s.handleError(c, l, 500);
		return 1;
	}

	if (pipe(pipeFromCgi) == -1)
	{
		std::cerr << "pipeFromCgi failed: "
				  << strerror(errno) << std::endl;

		close(pipeToCgi[0]);
		close(pipeToCgi[1]);

		s.handleError(c, l, 500);
		return 1;
	}

	pid_t pid = fork();

	if (pid == -1)
	{
		std::cerr << "fork failed: "
				  << strerror(errno) << std::endl;

		close(pipeToCgi[0]);
		close(pipeToCgi[1]);
		close(pipeFromCgi[0]);
		close(pipeFromCgi[1]);

		s.handleError(c, l, 500);
		return 1;
	}

	if (pid == 0)
	{
		close(pipeToCgi[1]);
		close(pipeFromCgi[0]);

		if (dup2(pipeToCgi[0], STDIN_FILENO) == -1)
			_exit(1);

		if (dup2(pipeFromCgi[1], STDOUT_FILENO) == -1)
			_exit(1);

		close(pipeToCgi[0]);
		close(pipeFromCgi[1]);

		execve(argv[0], argv, envp.data());

		/*
		 * The parent will detect this through waitpid()
		 * inside receiveFromCgi().
		 */
		_exit(127);
	}

	/*
	 * Parent
	 */
	close(pipeToCgi[0]);
	close(pipeFromCgi[1]);

	c.cgiInputFd = pipeToCgi[1];
	c.cgiOutputFd = pipeFromCgi[0];

	c.cgiBody = c.request.body;
	c.cgiBodyOffset = 0;

	c.cgiResponse.clear();
	c.cgiPid = pid;

	if (fcntl(c.cgiInputFd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "fcntl CGI input failed: "
				  << strerror(errno) << std::endl;

		close(c.cgiInputFd);
		close(c.cgiOutputFd);

		c.cgiInputFd = -1;
		c.cgiOutputFd = -1;

		s.handleError(c, l, 500);
		return 1;
	}

	if (fcntl(c.cgiOutputFd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "fcntl CGI output failed: "
				  << strerror(errno) << std::endl;

		close(c.cgiInputFd);
		close(c.cgiOutputFd);

		c.cgiInputFd = -1;
		c.cgiOutputFd = -1;

		s.handleError(c, l, 500);
		return 1;
	}

	pollfd stdinCgi;

	stdinCgi.fd = c.cgiInputFd;
	stdinCgi.events = POLLOUT;
	stdinCgi.revents = 0;

	s.pollfds_vector.push_back(stdinCgi);

	pollfd stdoutCgi;

	stdoutCgi.fd = c.cgiOutputFd;
	stdoutCgi.events = POLLIN;
	stdoutCgi.revents = 0;

	s.pollfds_vector.push_back(stdoutCgi);

	return 1;
}


/*int	method_HEAD( Client &c, Server &s, int l )
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
}*/


int method_HEAD(Client &c, Server &s, int l)
{
	HTTPresponse response;
	Location location = s.serversConfs.getLocations()[l];

	std::string path(location.getPagePath());

	struct stat fileInfo;

	if (stat(path.c_str(), &fileInfo) == -1)
	{
		std::cerr << "stat failed: "
				  << strerror(errno) << std::endl;

		if (errno == EACCES)
			s.handleError(c, l, 403);
		else
			s.handleError(c, l, 404);

		return 1;
	}

	if (S_ISDIR(fileInfo.st_mode))
	{
		s.handleError(c, l, 403);
		return 1;
	}

	std::stringstream ss;
	ss << fileInfo.st_size;

	std::vector<std::pair<std::string, std::string> > headers;

	headers.push_back(
		std::make_pair("Content-Length", ss.str())
	);

	headers.push_back(
		std::make_pair("Content-Type", "text/html")
	);

	response.setStatusCode(200);
	response.setHeaders(headers);

	c.sendBuffer = response.buildResponse();
	c.sendOffset = 0;

	s.enableClientWrite(c.fd);

	return 1;
}


int method_OPTIONS(Client &c, Server &s, int l)
{
	if (l < 0)
	{
		s.handleError(c, l, 404);
		return 1;
	}

	HTTPresponse response;
	Location location = s.serversConfs.getLocations()[l];

	const std::string *allowedMethods =
		location.getAllowedMethods();

	std::string allow;

	for (size_t i = 0; i < 9; ++i)
	{
		if (allowedMethods[i].empty())
			break;

		if (!allow.empty())
			allow += ", ";

		allow += allowedMethods[i];
	}

	std::vector<std::pair<std::string, std::string> > headers;

	headers.push_back(
		std::make_pair("Allow", allow)
	);

	headers.push_back(
		std::make_pair("Content-Length", "0")
	);

	response.setStatusCode(204);
	response.setHeaders(headers);

	c.sendBuffer = response.buildResponse();
	c.sendOffset = 0;

	s.enableClientWrite(c.fd);

	return 1;
}


int method_TRACE(Client &c, Server &s, int l)
{
	if (l < 0)
	{
		s.handleError(c, l, 404);
		return 1;
	}

	HTTPresponse response;
	std::string resBody;
	std::map<std::string, std::string>::const_iterator it;

	resBody = c.request.method + " "
			+ c.request.path + " "
			+ c.request.version + "\r\n";

	for (it = c.request.headers.begin();
		 it != c.request.headers.end();
		 ++it)
	{
		resBody += it->first + ": "
				+ it->second + "\r\n";
	}

	resBody += "\r\n" + c.request.body;

	std::vector<std::pair<std::string, std::string> > headers;
	std::stringstream ss;

	ss << resBody.size();

	headers.push_back(
		std::make_pair("Content-Type", "message/http")
	);

	headers.push_back(
		std::make_pair("Content-Length", ss.str())
	);

	response.setStatusCode(200);
	response.setHeaders(headers);
	response.setBody(resBody);

	c.sendBuffer = response.buildResponse();
	c.sendOffset = 0;

	s.enableClientWrite(c.fd);

	return 1;
}



int	method_PATCH(Client &c, Server &s, int l)
{
	if (l < 0)
	{
		s.handleError(c, l, 404);
		return 1;
	}

	Location location = s.serversConfs.getLocations()[l];

	std::string p = c.request.path;
	std::string postfix;

	for (size_t i = 0; i < p.size(); ++i)
	{
		if (p[i] == '.')
		{
			postfix = p.substr(i);
			break;
		}
	}

	std::vector<std::pair<std::string, std::string> > cgis =
		location.getCgi();

	size_t j = 0;

	for (; j < cgis.size(); ++j)
	{
		if (postfix == cgis[j].first)
			break;
	}

	if (j == cgis.size())
	{
		s.handleError(c, l, 500);
		return 1;
	}

	std::string compiler = findCGIcompiler(cgis[j].first);
	std::string script = cgis[j].second;

	char *argv[3];

	argv[0] = const_cast<char *>(compiler.c_str());
	argv[1] = const_cast<char *>(script.c_str());
	argv[2] = NULL;

	std::vector<std::string> tempEnvp =
		buildEnvironment(c, s, script);

	std::vector<char *> envp;

	for (size_t k = 0; k < tempEnvp.size(); ++k)
	{
		envp.push_back(
			const_cast<char *>(tempEnvp[k].c_str())
		);
	}

	envp.push_back(NULL);

	int pipeToCgi[2];
	int pipeFromCgi[2];

	if (pipe(pipeToCgi) == -1)
	{
		std::cerr << "pipeToCgi failed: "
				  << strerror(errno) << std::endl;

		s.handleError(c, l, 500);
		return 1;
	}

	if (pipe(pipeFromCgi) == -1)
	{
		std::cerr << "pipeFromCgi failed: "
				  << strerror(errno) << std::endl;

		close(pipeToCgi[0]);
		close(pipeToCgi[1]);

		s.handleError(c, l, 500);
		return 1;
	}

	pid_t pid = fork();

	if (pid == -1)
	{
		std::cerr << "fork failed: "
				  << strerror(errno) << std::endl;

		close(pipeToCgi[0]);
		close(pipeToCgi[1]);
		close(pipeFromCgi[0]);
		close(pipeFromCgi[1]);

		s.handleError(c, l, 500);
		return 1;
	}

	if (pid == 0)
	{
		close(pipeToCgi[1]);
		close(pipeFromCgi[0]);

		if (dup2(pipeToCgi[0], STDIN_FILENO) == -1)
			_exit(1);

		if (dup2(pipeFromCgi[1], STDOUT_FILENO) == -1)
			_exit(1);

		close(pipeToCgi[0]);
		close(pipeFromCgi[1]);

		execve(argv[0], argv, envp.data());

		perror("execve");
		_exit(127);
	}

	close(pipeToCgi[0]);
	close(pipeFromCgi[1]);

	c.cgiInputFd = pipeToCgi[1];
	c.cgiOutputFd = pipeFromCgi[0];

	c.cgiBody = c.request.body;
	c.cgiBodyOffset = 0;
	c.cgiResponse.clear();
	c.cgiPid = pid;

	if (fcntl(c.cgiInputFd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "fcntl CGI input failed: "
				  << strerror(errno) << std::endl;

		close(c.cgiInputFd);
		close(c.cgiOutputFd);

		c.cgiInputFd = -1;
		c.cgiOutputFd = -1;

		s.handleError(c, l, 500);
		return 1;
	}

	if (fcntl(c.cgiOutputFd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "fcntl CGI output failed: "
				  << strerror(errno) << std::endl;

		close(c.cgiInputFd);
		close(c.cgiOutputFd);

		c.cgiInputFd = -1;
		c.cgiOutputFd = -1;

		s.handleError(c, l, 500);
		return 1;
	}

	pollfd stdinCgi;

	stdinCgi.fd = c.cgiInputFd;
	stdinCgi.events = POLLOUT;
	stdinCgi.revents = 0;

	s.pollfds_vector.push_back(stdinCgi);

	pollfd stdoutCgi;

	stdoutCgi.fd = c.cgiOutputFd;
	stdoutCgi.events = POLLIN;
	stdoutCgi.revents = 0;

	s.pollfds_vector.push_back(stdoutCgi);

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

	std::cout << "connectUpstream returned FD: "
          << upstreamFd << std::endl;

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
    pollfd upstreamPollFd;
    upstreamPollFd.fd = upstreamFd;
    upstreamPollFd.events = POLLIN;
    upstreamPollFd.revents = 0;
    s.pollfds_vector.push_back(upstreamPollFd);

    c.upstreamfd = upstreamFd;
    c.tunnel = true;

    return 0;
}

void	processRequest(Client &c, Server &s)
{

	int location = s.findLocation(c);

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
	if (c.request.path == "/connect-message" && c.request.method == "POST")
	{
		method_CONNECTMessage(c, s);
		return ;
	}
	if (c.request.path == "/connect-status" && c.request.method == "GET")
	{
		method_CONNECTStatus(c, s);
		return ;
	}

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
			
		s.handleError(c, location, 405);
		return ;
	}

    if (c.request.method == "POST" ||
        c.request.method == "PUT"  ||
        c.request.method == "PATCH"||
        c.request.method == "DELETE")
    {
        s.handleSession(c);
    }

	std::string methods[] = {
		"GET",
		"POST",
		"DELETE",
		"PUT",
		"HEAD",
		"OPTIONS",
		"TRACE",
		"PATCH",
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
	// print_info(c.request);
}