/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestSupport.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: biphuyal <biphuyal@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/24 15:57:33 by hgutterr          #+#    #+#             */
/*   Updated: 2026/10/28 17:17:20 by biphuyal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <unistd.h>
#include <HTTPrequest.hpp>
#include <HTTPresponse.hpp>
#include <Server.hpp>
#include <RequestSupport.hpp>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>


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

std::string buildEnvVariavle(const std::string &name, const std::string &value)
{
    std::string newName = convertToUpperCase(name);

    for (size_t i = 0; i < newName.size(); ++i)
        if (newName[i] == '-')
            newName[i] = '_';

    if (newName != "CONTENT_TYPE" && newName != "CONTENT_LENGTH")
        newName = "HTTP_" + newName;

    return newName + "=" + value;
}

std::vector<std::string> buildEnvironment(const Client &c, const Server &s, std::string execLoc)
{
    std::vector<std::string> enviorment;
    std::map<std::string, std::string>::const_iterator it;
	char resolvedPath[PATH_MAX];

    enviorment.push_back("REQUEST_METHOD=" + c.request.method);
    enviorment.push_back("SERVER_PROTOCOL=" + c.request.version);
    enviorment.push_back("SERVER_NAME=" + s.serversConfs.getServerName());

    std::stringstream ss;
    ss << s.serversConfs.getListenPort();

    enviorment.push_back("SERVER_PORT=" + ss.str());
    enviorment.push_back("GATEWAY_INTERFACE=CGI/1.1");

	int locationIndex = s.findLocation(c);
	if (locationIndex >= 0)
	{
		std::string documentRoot =
			s.serversConfs.getLocations()[locationIndex].getPagePath();
		if (!documentRoot.empty())
		{
			char resolvedRoot[PATH_MAX];
			if (realpath(documentRoot.c_str(), resolvedRoot) != NULL)
				documentRoot = resolvedRoot;
			enviorment.push_back("DOCUMENT_ROOT=" + documentRoot);
		}
	}

	std::string scriptName = c.request.path;
	std::string queryString;
	size_t queryPosition = scriptName.find('?');
	if (queryPosition != std::string::npos)
	{
		queryString = scriptName.substr(queryPosition + 1);
		scriptName.erase(queryPosition);
	}

	enviorment.push_back("SCRIPT_NAME=" + scriptName);
	enviorment.push_back("PATH_INFO=" + scriptName);
	enviorment.push_back("REQUEST_URI=" + scriptName);
	enviorment.push_back("QUERY_STRING=" + queryString);
	if (realpath(execLoc.c_str(), resolvedPath) != NULL)
		enviorment.push_back("SCRIPT_FILENAME=" + std::string(resolvedPath));
	else
		enviorment.push_back("SCRIPT_FILENAME=" + execLoc);

    // Required by some PHP CGI builds
    enviorment.push_back("REDIRECT_STATUS=1");

	bool hasCookieHeader = false;
	for (it = c.request.headers.begin();
         it != c.request.headers.end();
         ++it)
    {
		if (it->first == "Cookie")
			hasCookieHeader = true;
        enviorment.push_back(
            buildEnvVariavle(
                convertToUpperCase(it->first),
                it->second
            )
        );
    }

	if (!hasCookieHeader && !c.sessionId.empty())
		enviorment.push_back("HTTP_COOKIE=SessionId=" + c.sessionId);

	std::cout << "SCRIPT_FILENAME=" << execLoc << std::endl;

    return enviorment;
}

int sendCGIResponse(Client &c, Server &s, const std::string &cgiResponse)
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

	int statusCode = 200;

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

		if (name == "Status")
		{
			std::stringstream statusStream(value);
			statusStream >> statusCode;
			continue;
		}

		if (name == "Content-Length")
			continue;

		headers.push_back(
			std::make_pair(name, value)
		);
	}

	std::stringstream ss;
	ss << body.size();

	headers.push_back(
		std::make_pair("Content-Length", ss.str())
	);

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
			std::make_pair("Content-Type", "text/plain")
		);
	}

	if (c.newSession)
	{
		headers.push_back(
			std::make_pair(
				"Set-Cookie",
				std::string("SessionId=")
				+ c.sessionId
				+ std::string("; Path=/")
			)
		);

		c.newSession = false;
	}

	response.setStatusCode(statusCode);
	response.setBody(body);
	response.setHeaders(headers);

	std::string responseStr = response.buildResponse();

	 c.sendBuffer = responseStr;
	 c.sendOffset = 0;
	 s.enableClientWrite(c.fd);

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

// Content types for the static website and uploaded files.
std::string getContentType(const std::string &path)
{
    size_t dot = path.find_last_of('.');
    std::string extension = dot == std::string::npos ? "" : path.substr(dot);
    for (size_t i = 0; i < extension.size(); ++i)
        extension[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(extension[i])));
    static const struct {
	const char *extension;
	const char *contentType;
		} types[] = {
		{".html", "text/html; charset=utf-8"},
		{".htm",  "text/html; charset=utf-8"},
		{".css",  "text/css; charset=utf-8"},
		{".js",   "text/javascript; charset=utf-8"},
		{".mjs",  "text/javascript; charset=utf-8"},
		{".json", "application/json"},
		{".txt",  "text/plain; charset=utf-8"},
		{".md",   "text/plain; charset=utf-8"},
		{".jpg",  "image/jpeg"},
		{".jpeg", "image/jpeg"},
		{".png",  "image/png"},
		{".gif",  "image/gif"},
		{".webp", "image/webp"},
		{".svg",  "image/svg+xml"},
		{".ico",  "image/x-icon"},
		{".pdf",  "application/pdf"},
	};

	for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); ++i)
	{
		if (extension == types[i].extension)
			return types[i].contentType;
	}
	return "application/octet-stream";
}

static int hexDigit(char value)
{
    if (value >= '0' && value <= '9')
		return value - '0';
    if (value >= 'a' && value <= 'f')
		return value - 'a' + 10;
    if (value >= 'A' && value <= 'F')
		return value - 'A' + 10;
    return -1;
}

std::string buildFilePath(const Location &location, const std::string &requestPath)
{
    std::string root = location.getPagePath();
    std::string path = requestPath.substr(0, requestPath.find('?'));
    if (path.size() < location.getPath().size())
		return "";
    std::string remaining = path.substr(location.getPath().size());
    std::string decoded;

    // Decode once. Reject invalid escapes and directory traversal components.
    for (size_t i = 0; i < remaining.size(); ++i)
    {
        unsigned char value = static_cast<unsigned char>(remaining[i]);
        if (value == '%')
        {
            if (i + 2 >= remaining.size())
				return "";
            int high = hexDigit(remaining[i + 1]);
            int low = hexDigit(remaining[i + 2]);
            if (high < 0 || low < 0)
				return "";
            value = static_cast<unsigned char>(high * 16 + low);
            i += 2;
        }
        if (value < 32 || value == 127 || value == '\\')
			return "";
        decoded += static_cast<char>(value);
    }
    std::stringstream components(decoded);
    std::string component;
    while (std::getline(components, component, '/'))
        if (component == "." || component == "..")
			return "";

    if (decoded.empty())
		return root;
    if (decoded[0] == '/') decoded.erase(0, 1);
    if (!root.empty() && root[root.size() - 1] != '/') root += '/';
    return root + decoded;
}

static std::string escapeJsonString(const std::string &value)
{
    const char *hex = "0123456789abcdef";
    std::string escaped;
    for (size_t i = 0; i < value.size(); ++i)
    {
        unsigned char c = static_cast<unsigned char>(value[i]);
        if (c == '"' || c == '\\')
        {
            escaped += '\\';
            escaped += static_cast<char>(c);
        }
        else if (c < 32)
        {
            escaped += "\\u00";
            escaped += hex[c >> 4];
            escaped += hex[c & 15];
        }
        else escaped += static_cast<char>(c);
    }
    return escaped;
}


int getFilesFolder(Client &c, Server &s, HTTPresponse &response, const std::string &path)
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
		body += escapeJsonString(filename);
		body += "\"";

		first = false;
	}

	closedir(dir);

	body += "]";

	std::stringstream ss;
	ss << body.size();

	std::vector<std::pair<std::string, std::string> > headers;

	headers.push_back(
		std::make_pair("Content-Length", ss.str())
	);

	headers.push_back(
		std::make_pair("Content-Type", "application/json")
	);

	response.setStatusCode(200);
	response.setBody(body);
	response.setHeaders(headers);

	c.sendBuffer = response.buildResponse();
	c.sendOffset = 0;

	s.enableClientWrite(c.fd);

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

std::string findExecutable(const std::string& name)
{
    const char *path = std::getenv("PATH");
    if (!path)
        return "";

    std::string paths(path);
    size_t start = 0;

    while (start < paths.size())
    {
        size_t end = paths.find(':', start);
        if (end == std::string::npos)
            end = paths.size();

        std::string dir = paths.substr(start, end - start);
        std::string fullPath = dir + "/" + name;

        if (access(fullPath.c_str(), X_OK) == 0)
            return fullPath;

        start = end + 1;
    }

    return "";
}

std::string findCGIcompiler(const std::string& extension)
{
    if (extension == ".py")
        return findExecutable("python3");

    if (extension == ".php")
		return findExecutable("php-cgi");

    return "";
}
