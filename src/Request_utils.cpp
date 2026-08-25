/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request_utils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/24 15:57:33 by hgutterr          #+#    #+#             */
/*   Updated: 2026/08/25 20:14:50 by hgutterr         ###   ########.fr       */
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


std::string convertToUpperCase(std::string text)
{
    for (size_t i = 0; i < text.size(); ++i)
    {
        if (text[i] >= 'a' && text[i] <= 'z')
            text[i] = text[i] - 'a' + 'A';

        if (text[i] == '-')
            text[i] = '_';
    }

    return text;
}

std::string buildEnvVariavle(
const std::string &name,
    const std::string &value)
{
    std::string newName = convertToUpperCase(name);

    if (newName != "CONTENT_TYPE" &&
        newName != "CONTENT_LENGTH")
    {
        newName = "HTTP_" + newName;
    }

    return newName + "=" + value;
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

    // Required by some PHP CGI builds
    enviorment.push_back("REDIRECT_STATUS=1");

    for (it = c.request.headers.begin();
         it != c.request.headers.end();
         ++it)
    {
        enviorment.push_back(
            buildEnvVariavle(
                convertToUpperCase(it->first),
                it->second
            )
        );
    }

	std::cout << "SCRIPT_FILENAME=" << execLoc << std::endl;

    return enviorment;
}

int sendCGIResponse(const Client &c, const std::string &cgiResponse)
{
	HTTPresponse response;

	std::cout << "PRETOOOOOOOO" << std::endl;

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

int checkIPaddress( std::string ip )
{
	if(ip.find_first_of(':') != ip.find_last_of(':'))
		return 1;
		
	std::string nums = ip.substr(ip.find_first_of(':'), ip.size() - ip.find_first_of(':'));

	for(size_t i = 0; i < nums.size(); ++i)
	{
		if(!(std::isdigit(nums[i])))
			return 1;
	}
	return 0;
}

std::string findCGIcompiler( std::string cgitype )
{
	if( cgitype == ".py" )
		return (std::string("/usr/bin/python3"));
	if ( cgitype == ".php" )
		return (std::string("/usr/bin/php-cgi"));
	return ("pila");
}
