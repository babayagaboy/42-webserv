/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 14:19:37 by hgutterr          #+#    #+#             */
/*   Updated: 2026/09/16 16:21:02 by myivanov         ###   ########.fr       */
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
int sendCGIResponse(Client &c, Server &s, const std::string &cgiResponse);
std::string buildFilePath(const Location &location, const std::string &requestPath);
int getFilesFolder(Client &c, Server &s, HTTPresponse &response, const std::string &path);
int checkIPaddress( std::string ip );
std::string findCGIcompiler(const std::string& extension);
void    print_info(const HTTPrequest &obj);

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

*/

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
		HTTPresponse response;
		std::vector<std::pair<std::string, std::string> > headers;
		std::stringstream length;
		length << c.request.body.size();
		headers.push_back(std::make_pair("Content-Length", length.str()));
		headers.push_back(std::make_pair("Content-Type", "text/plain"));
		response.setStatusCode(200);
		response.setHeaders(headers);
		response.setBody(c.request.body);
		c.sendBuffer = response.buildResponse();
		c.sendOffset = 0;
		s.enableClientWrite(c.fd);
		return 1;
	}

	std::string compiler = findCGIcompiler(cgis[j].first);
	std::string script = cgis[j].second;
	bool directCgi = compiler.empty();
	if (directCgi)
		compiler = script;

	char *argv[3];

	argv[0] = const_cast<char *>(compiler.c_str());
	argv[1] = directCgi ? NULL : const_cast<char *>(script.c_str());
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
	 c.cgiStart = std::time(NULL);

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
		std::string filePath = buildFilePath(location, c.request.path);
		struct stat fileInfo;
		if (stat(filePath.c_str(), &fileInfo) == 0)
		{
			if (S_ISDIR(fileInfo.st_mode))
			{
				s.handleError(c, l, 403);
				return 1;
			}
			if (unlink(filePath.c_str()) == 0)
			{
				HTTPresponse response;
				response.setStatusCode(204);
				response.setHeaders(std::vector<std::pair<std::string, std::string> >());
				c.sendBuffer = response.buildResponse();
				c.sendOffset = 0;
				s.enableClientWrite(c.fd);
				return 1;
			}
			s.handleError(c, l, errno == EACCES ? 403 : 500);
			return 1;
		}
		s.handleError(c, l, errno == EACCES ? 403 : 404);
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
	 c.cgiStart = std::time(NULL);

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
	 c.cgiStart = std::time(NULL);

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

  #if 0
	  if (pipe(pipeFromCgi) == -1)
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
	 c.cgiStart = std::time(NULL);

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

	#endif

	int method_GET(Client &c, Server &s, int l)
	{
		HTTPresponse response;
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
		const std::vector<std::pair<std::string, std::string> > &cgis =
			location.getCgi();
		for (size_t i = 0; i < cgis.size(); ++i)
		{
			if (postfix == cgis[i].first
				&& (cgis[i].first == ".py" || cgis[i].first == ".php"))
				return method_POST(c, s, l);
		}
		std::string path = buildFilePath(location, c.request.path);
		struct stat pathStat;

		if (stat(path.c_str(), &pathStat) == -1)
		{
			s.handleError(c, l, 404);
			return 1;
		}
		if (S_ISDIR(pathStat.st_mode))
		{
			if (!location.getIndex().empty())
			{
				std::string indexPath = path;
				if (indexPath[indexPath.size() - 1] != '/')
					indexPath += '/';
				indexPath += location.getIndex();
				if (stat(indexPath.c_str(), &pathStat) == 0 && !S_ISDIR(pathStat.st_mode))
					path = indexPath;
				else if (location.getAutoIndex())
					return getFilesFolder(c, s, response, path);
				else
				{
					s.handleError(c, l, 404);
					return 1;
				}
			}
			else if (location.getAutoIndex())
				return getFilesFolder(c, s, response, path);
			else
			{
				s.handleError(c, l, 404);
				return 1;
			}
		}

		int fd = open(path.c_str(), O_RDONLY);
		if (fd < 0)
		{
			s.handleError(c, l, errno == EACCES ? 403 : 500);
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
			s.handleError(c, l, 500);
			return 1;
		}

		std::stringstream length;
		length << body.size();
		std::vector<std::pair<std::string, std::string> > headers;
		headers.push_back(std::make_pair("Content-Length", length.str()));
		headers.push_back(std::make_pair("Content-Type", "text/html"));
		response.setStatusCode(200);
		response.setHeaders(headers);
		response.setBody(body);
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
		const std::string *allowed = s.serversConfs.getLocations()[l].getAllowedMethods();
		std::string allow;
		for (size_t i = 0; i < 9 && !allowed[i].empty(); ++i)
		{
			if (!allow.empty())
				allow += ", ";
			allow += allowed[i];
		}
		HTTPresponse response;
		std::vector<std::pair<std::string, std::string> > headers;
		headers.push_back(std::make_pair("Allow", allow));
		headers.push_back(std::make_pair("Content-Length", "0"));
		response.setStatusCode(204);
		response.setHeaders(headers);
		c.sendBuffer = response.buildResponse();
		c.sendOffset = 0;
		s.enableClientWrite(c.fd);
		return 1;
	}

	int method_PATCH(Client &c, Server &s, int l)
	{
		return method_POST(c, s, l);
	}

	int method_HEAD(Client &c, Server &s, int l)
{
		Location location = s.serversConfs.getLocations()[l];
		std::string path = buildFilePath(location, c.request.path);
		struct stat fileInfo;

		if (stat(path.c_str(), &fileInfo) == -1)
		{
				s.handleError(c, l, 404);
				return 1;
		}
			if (S_ISDIR(fileInfo.st_mode))
			{
				std::string indexPath = path;
				if (indexPath[indexPath.size() - 1] != '/')
						indexPath += '/';
				indexPath += location.getIndex();
				if (location.getIndex().empty()
						|| stat(indexPath.c_str(), &fileInfo) != 0
						|| S_ISDIR(fileInfo.st_mode))
				{
						s.handleError(c, l, 403);
						return 1;
				}
			}

		std::stringstream length;
		length << fileInfo.st_size;
		std::vector<std::pair<std::string, std::string> > headers;
		headers.push_back(std::make_pair("Content-Length", length.str()));
		headers.push_back(std::make_pair("Content-Type", "text/html"));

		HTTPresponse response;
		response.setStatusCode(200);
		response.setHeaders(headers);
		c.sendBuffer = response.buildResponse();
		c.sendOffset = 0;
		s.enableClientWrite(c.fd);
		return 1;
}

	void processRequest(Client &c, Server &s)
{
	int location = s.findLocation(c);
	if (location < 0)
	{
		s.handleError(c, -1, 404);
		return;
	}

	const std::vector<std::pair<int, std::string> > &returns =
		s.serversConfs.getLocations()[location].getReturn();
	if (!returns.empty())
	{
		HTTPresponse response;
		std::vector<std::pair<std::string, std::string> > headers;
		if (!returns[0].second.empty())
			headers.push_back(std::make_pair("Location", returns[0].second));
		headers.push_back(std::make_pair("Content-Length", "0"));
		response.setStatusCode(returns[0].first);
		response.setHeaders(headers);
		c.sendBuffer = response.buildResponse();
		c.sendOffset = 0;
		s.enableClientWrite(c.fd);
		return;
	}

	if (!s.isMethodAllowed(c.request.method, location))
	{
		s.handleError(c, location, 405);
		return;
	}

	if (c.request.method == "POST" || c.request.method == "PUT"
		|| c.request.method == "PATCH" || c.request.method == "DELETE")
		s.handleSession(c);

	std::string methods[] = {
		"GET", "POST", "DELETE", "PUT", "HEAD", "OPTIONS", "PATCH"
	};
	int (*methodFunctions[])(Client &, Server &, int) = {
		&method_GET, &method_POST, &method_DELETE, &method_PUT,
		&method_HEAD, &method_OPTIONS, &method_PATCH
	};
	for (size_t i = 0; i < 7; ++i)
	{
		if (c.request.method == methods[i])
		{
			methodFunctions[i](c, s, location);
			return;
		}
	}
	s.handleError(c, location, 405);
}