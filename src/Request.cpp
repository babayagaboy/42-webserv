/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 14:19:37 by hgutterr          #+#    #+#             */
/*   Updated: 2026/08/16 21:07:16 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <unistd.h>
#include <HTTPrequest.hpp>
#include <HTTPresponse.hpp>
#include <Server.hpp>
#include <sys/types.h>
#include <sys/wait.h>


// HTTP/1.1 200 OK\r\n
// Content-Type: text/html; charset=UTF-8\r\n
// Date: Fri, 21 Jun 2024 14:18:33 GMT\r\n
// Last-Modified: Thu, 17 Oct 2019 07:18:26 GMT\r\n
// Content-Length: 1234\r\n
// \r\n

void    print_info(const HTTPrequest &obj)
{
	std::cout << std::endl << "HTTPrequest method: " << obj.method  << std::endl;
	std::cout << "HTTPrequest content: " << obj.path  << std::endl;
	std::cout << "HTTPrequest version: " << obj.version  << std::endl << std::endl;

	std::map<std::string, std::string>::const_iterator it;

	for (it = obj.headers.begin(); it != obj.headers.end(); ++it) {
		std::cout << it->first << ": " << it->second << std::endl;
	}

	std::cout << "Body: " << obj.body << std::endl;
}



int	method_GET( const Client &c, const Server &s, int l )
{
	HTTPresponse response;
	Location location = s.serversConfs.getLocations()[l];
	std::string path(location.getPagePath());
	
	std::cout << "PATH: " << path << std::endl;
	int fd = open(path.c_str(), O_RDONLY);
	std::cout << "good old goy2" << std::endl;

	if (fd < 0)
		return 0;

	char buffer[4096];
	std::string body;
	ssize_t bytesRead;

	std::cout << "good old goy1" << std::endl;

	while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
		body.append(buffer, bytesRead);
	close(fd);

	std::vector<std::pair<std::string, std::string> > headers;

	std::stringstream ss;

	ss << body.size();
	std::string bodySize = ss.str();

	headers.push_back(std::make_pair("Content-Length", bodySize));
	headers.push_back(std::make_pair("Content-Type", "text/html"));

	
	response.setStatusCode(200);
	response.setBody(body);
	response.setHeaders(headers);
	
	std::string responseStr = response.buildResponse();
	
	std::cout << "good old goy" << std::endl;
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

/*std::string	buildCGIVariable(const std::string &name, const std::string &value)
{

}*/


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

	std::cout << "POSTFIX IS: " << postfix << std::endl;
	std::cout << "LOCATION IS THIS: " << std::endl;
	std::cout << location << std::endl;
	
	std::vector<std::pair<std::string, std::string > > cgis = location.getCgi();
	

	size_t j = 0;
	for (; j < cgis.size(); ++j) {
		std::cout << "CGIS[" << 0 << "].first = " << cgis[0].first << std::endl;
		if (postfix == cgis[j].first)
			break ;
	}
	

	std::cout << "SIZE OF CGIs is : " <<  cgis.size() << " AND J IS: " << j << std::endl;

	if (j == cgis.size())
    	return -1;

	//argv[0] = const_cast<char *>(cgis[j].second.c_str());
	argv[0] = const_cast<char *>("/usr/bin/python3");
	argv[1] = const_cast<char *>(cgis[j].second.c_str());
	argv[2] = NULL;

	std::cout << "POSTFIX SLIMED" << std::endl;


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
	else {

		std::cout << "this nga posted :)" << std::endl;

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

		std::cout << "========== CGI RESPONSE ==========" << std::endl;
		std::cout << cgiResponse << std::endl;
		std::cout << "==================================" << std::endl;

		waitpid(pid, NULL, 0);
	}
	
	return 1;
}

int	method_DELETE( const Client &c, const Server &s, int l )
{
	(void)c;
	(void)s;
	(void)l;
	return 1;
}

int	method_PUT( const Client &c, const Server &s, int l )
{
	(void)c;
	(void)s;
	(void)l;
	return 1;
}

int	method_HEAD( const Client &c, const Server &s, int l )
{
	(void)c;
	(void)s;
	(void)l;
	return 1;
}

int	method_OPTIONS( const Client &c, const Server &s, int l )
{
	(void)c;
	(void)s;
	(void)l;
	return 1;
}

int	method_TRACE( const Client &c, const Server &s, int l )
{
	(void)c;
	(void)s;
	(void)l;
	return 1;
}

int	method_CONNECT( const Client &c, const Server &s, int l )
{
	(void)c;
	(void)s;
	(void)l;
	return 1;
}

int	method_PATCH( const Client &c, const Server &s, int l )
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

	for (size_t i = 0; i < methods->size(); ++i)
	{
		if(c.request.method == methods[i])
			methfunctions[i](c, s, location);
	}
	
	print_info(c.request);
}