/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 14:19:37 by hgutterr          #+#    #+#             */
/*   Updated: 2026/08/21 23:07:06 by hgutterr         ###   ########.fr       */
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


// HTTP/1.1 200 OK\r\n
// Content-Type: text/html; charset=UTF-8\r\n
// Date: Fri, 21 Jun 2024 14:18:33 GMT\r\n
// Last-Modified: Thu, 17 Oct 2019 07:18:26 GMT\r\n
// Content-Length: 1234\r\n
// \r\n

int sendCGIResponse(const Client &c, const std::string &cgiResponse)
{
	HTTPresponse response;

	std::string headersPart;
	std::string body;
	size_t separator = cgiResponse.find("\r\n\r\n");

	if (separator != std::string::npos)
	{
		headersPart = cgiResponse.substr(0, separator);
		body = cgiResponse.substr(separator + 4);
	}
	else
	{
		separator = cgiResponse.find("\n\n");

		if (separator != std::string::npos)
		{
			headersPart = cgiResponse.substr(0, separator);
			body = cgiResponse.substr(separator + 2);
		}
		else
			body = cgiResponse;
	}

	std::vector<std::pair<std::string, std::string> > headers;
	std::stringstream headerStream(headersPart);
	std::string line;

	while (std::getline(headerStream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		size_t colon = line.find(':');

		if (colon == std::string::npos)
			continue;

		std::string name = line.substr(0, colon);
		std::string value = line.substr(colon + 1);
		while (!value.empty() && value[0] == ' ')
			value.erase(0, 1);

		if (name == "Content-Length")
			continue;
		if (name == "Status")
			continue;

		headers.push_back(std::make_pair(name, value));
	}

	std::stringstream ss;
	ss << body.size();

	headers.push_back(
		std::make_pair("Content-Length", ss.str()));

	bool hasContentType = false;

	for (size_t i = 0; i < headers.size(); ++i)
	{
		if (headers[i].first == "Content-Type")
		{
			hasContentType = true;
			break;
		}
	}

	if (!hasContentType)
	{
		headers.push_back(
			std::make_pair("Content-Type", "text/plain"));
	}

	response.setStatusCode(200);
	response.setBody(body);
	response.setHeaders(headers);

	std::string responseStr = response.buildResponse();

	std::cout << responseStr << std::endl;

	if (send(c.fd, responseStr.c_str(), responseStr.size(), 0) < 0)
	{
		perror("send");
		return -1;
	}

	return 1;
}


// void    print_info(const HTTPrequest &obj)
// {
// 	std::cout << std::endl << "HTTPrequest method: " << obj.method  << std::endl;
// 	std::cout << "HTTPrequest content: " << obj.path  << std::endl;
// 	std::cout << "HTTPrequest version: " << obj.version  << std::endl << std::endl;

// 	std::map<std::string, std::string>::const_iterator it;

// 	for (it = obj.headers.begin(); it != obj.headers.end(); ++it) {
// 		std::cout << it->first << ": " << it->second << std::endl;
// 	}

// 	std::cout << "Body: " << obj.body << std::endl;
// }

std::string buildFilePath(const Location &location, const std::string &requestPath)
{
	std::string root = location.getPagePath();
	std::string locationPath = location.getPath();

	if (requestPath == locationPath)
		return root;

	std::string remaining =
	requestPath.substr(locationPath.size());

	if (!remaining.empty() && remaining[0] == '/')
		remaining.erase(0, 1);

	if (!root.empty() && root[root.size() - 1] != '/')
		root += '/';

	return root + remaining;
}


int getFilesFolder(const Client &c, HTTPresponse &response, const std::string &path)
{
	DIR *dir = opendir(path.c_str());

	if (!dir)
	{
		perror("opendir");
		return -1;
	}

	std::string body = "[";
	struct dirent *entry;
	bool first = true;

	while ((entry = readdir(dir)) != NULL)
	{
		std::string filename = entry->d_name;

		if (filename == "." || filename == "..")
			continue;

		if (!first)
			body += ",";

		body += "\"";
		body += filename;
		body += "\"";

		first = false;
	}

	closedir(dir);

	body += "]";

	std::stringstream ss;
	ss << body.size();

	std::vector<std::pair<std::string, std::string> > headers;

	headers.push_back(
	std::make_pair("Content-Length", ss.str()));

	headers.push_back(
	std::make_pair("Content-Type", "application/json"));

	response.setStatusCode(200);
	response.setBody(body);
	response.setHeaders(headers);

	std::string responseStr = response.buildResponse();

	send(c.fd, responseStr.c_str(), responseStr.size(), 0);

	return 1;
}


int	method_GET(const Client &c, const Server &s, int l)
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




std::string	convertToUpperCase(std::string text)
{
	std::string result;

	for (size_t i = 0; i < text.size(); ++i)
	{
		if (text[i] == '-') {
			result.push_back('_');
			continue ;
		}

		result.push_back(text[i] - 32);
	}

	return result;
}

std::string buildEnvVariavle(const std::string &name, const std::string &value)
{
	std::string envVariable;

	std::string newName;

	if (name == "Content-Type")
		newName = "CONTENT_TYPE";
	else if (name == "Content-Length")
		newName = "CONTENT_LENGTH";
	else
		newName = "HTTP_" + convertToUpperCase(name);


	envVariable = newName + "=" + value;

	return envVariable;
}

std::vector<std::string> buildEnvironment(const Client &c, const Server &s, std::string execLoc)
{
	std::vector<std::string> enviorment;
	std::map<std::string, std::string>::const_iterator it;


	enviorment.push_back("REQUEST_METHOD=" + c.request.method);
	enviorment.push_back("SERVER_PROTOCOL=" + c.request.version);
	enviorment.push_back("SERVER_NAME=" + s.serversConfs.getServerName());

	std::stringstream ss;
	ss << s.serversConfs.getListenPort();

	enviorment.push_back("SERVER_PORT=" + ss.str());
	enviorment.push_back("GATEWAY_INTERFACE=CGI/1.1");
	enviorment.push_back("SCRIPT_NAME=" + c.request.path);
	enviorment.push_back("SCRIPT_FILENAME=" + execLoc);

	for (it = c.request.headers.begin(); it != c.request.headers.end(); ++it) {
		enviorment.push_back(buildEnvVariavle(convertToUpperCase(it->first), it->second));
	}

	return enviorment;

}

int	method_POST( const Client &c, const Server &s, int l )
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

		sendCGIResponse(c, cgiResponse);
	}
	return 1;
}

int	method_DELETE(const Client &c, const Server &s, int l)
{
	Location location = s.serversConfs.getLocations()[l];
	std::string path(location.getPagePath());
	std::string p(c.request.path);
	std::string postfix;

	char *argv[3];

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
		return -1;

	argv[0] = const_cast<char *>("/usr/bin/python3");
	argv[1] = const_cast<char *>(cgis[j].second.c_str());
	argv[2] = NULL;

	std::vector<std::string> tempEnvp =
		buildEnvironment(c, s, path);

	size_t i = tempEnvp.size();
	char *envp[i + 1];

	size_t k = 0;

	for (; k < tempEnvp.size(); ++k)
		envp[k] = const_cast<char *>(tempEnvp[k].c_str());

	envp[k] = NULL;

	int pipeToCgi[2];
	int pipeFromCgi[2];

	if (pipe(pipeToCgi) == -1)
	{
		std::cout << "PIPE ERROR: error while creating pipeToCgi"
				  << std::endl;
		return -1;
	}

	if (pipe(pipeFromCgi) == -1)
	{
		std::cout << "PIPE ERROR: error while creating pipeFromCgi"
				  << std::endl;
		close(pipeToCgi[0]);
		close(pipeToCgi[1]);
		return -1;
	}

	pid_t pid = fork();

	if (pid == -1)
	{
		std::cout << "FORK ERROR: error while creating child process"
				  << std::endl;
		return -1;
	}

	if (pid == 0)
	{
		close(pipeToCgi[1]);
		close(pipeFromCgi[0]);

		if (dup2(pipeToCgi[0], STDIN_FILENO) == -1)
			exit(1);

		if (dup2(pipeFromCgi[1], STDOUT_FILENO) == -1)
			exit(1);

		close(pipeToCgi[0]);
		close(pipeFromCgi[1]);

		execve(argv[0], argv, envp);

		perror("execve");
		exit(1);
	}
	else
	{
		close(pipeToCgi[0]);
		close(pipeFromCgi[1]);

		const std::string &body = c.request.body;

		if (write(pipeToCgi[1], body.c_str(), body.size()) == -1)
		{
			std::cout << "Error writing body to CGI"
					  << std::endl;
		}

		close(pipeToCgi[1]);

		char buffer[4096];
		std::string cgiResponse;
		ssize_t bytesRead;

		while ((bytesRead = read(pipeFromCgi[0], buffer, sizeof(buffer))) > 0)
			cgiResponse.append(buffer, bytesRead);

		close(pipeFromCgi[0]);
		waitpid(pid, NULL, 0);
		sendCGIResponse(c, cgiResponse);
	}

	return 1;
}

int	method_PUT( const Client &c, const Server &s, int l )
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

		sendCGIResponse(c, cgiResponse);
	}
	return 1;
}

int	method_HEAD( const Client &c, const Server &s, int l )
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

int	method_OPTIONS( const Client &c, const Server &s, int l )
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

int	method_TRACE( const Client &c, const Server &s, int l )
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


int	method_PATCH( const Client &c, const Server &s, int l )
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

int	method_CONNECT( const Client &c, const Server &s, int l )
{
	(void)c;
	(void)s;
	(void)l;
	return 1;
}





void	processRequest(const Client &c, const Server &s)
{
	int location = s.findLocation(c);

	if (location < 0)
	{
		std::cout << "No matching location" << std::endl;
		return;
	}

	if (!s.isMethodAllowed(c.request.method, location))
	{
		std::cout << "Method "
				  << c.request.method
				  << " is not allowed for location "
				  << s.serversConfs.getLocations()[location].getPath()
				  << std::endl;
		return;
	}

	std::string methods[] = {
		"GET",
		"POST",
		"DELETE",
		"PUT",
		"HEAD",
		"OPTIONS",
		"TRACE",
		"CONNECT",
		"PATCH"
	};
	
	int (*methfunctions[]) ( const Client &c, const Server &s, int location ) = {
		&method_GET, 
		&method_POST, 
		&method_DELETE,
        &method_PUT,
		&method_HEAD,
		&method_OPTIONS,
		&method_TRACE,
		&method_CONNECT,
		&method_PATCH
	};

	for (size_t i = 0; i < 9; ++i)
	{
		if(c.request.method == methods[i])
			methfunctions[i](c, s, location);
	}
	
	//print_info(c.request);
}