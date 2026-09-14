/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fillHTTPobject.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 15:19:33 by myivanov          #+#    #+#             */
/*   Updated: 2026/09/14 14:21:22 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <HTTPrequest.hpp>

static std::string lowerHeaderName(const std::string &name)
{
    std::string result = name;
    for (size_t i = 0; i < result.size(); ++i)
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
    return result;
}

static std::string canonicalHeaderName(const std::string &name)
{
    std::string lower = lowerHeaderName(name);
    if (lower == "content-length")
        return "Content-Length";
    if (lower == "content-type")
        return "Content-Type";
    if (lower == "transfer-encoding")
        return "Transfer-Encoding";
    if (lower == "cookie")
        return "Cookie";
    if (lower == "host")
        return "Host";
    return name;
}

void    rev_request_firstLine(HTTPrequest &obj, std::stringstream &ss)
{
	std::string firstLine;
	std::getline(ss, firstLine);
	std::stringstream lineStream(firstLine);

    // std::cout << "\n\nline : " << lineStream.str() << std::endl;

	lineStream >> obj.method;
	lineStream >> obj.path;
	lineStream >> obj.version;
}

void    rev_request_body(HTTPrequest &obj, std::stringstream &ss) {
    
    std::string body_text;

	while (std::getline(ss, body_text)) {
		obj.body.append(body_text);
	}
}

std::string&    str_trim(std::string &header_content) {
    
    for (unsigned long i = 0; i < header_content.size(); ++i) {
        if (std::isspace(static_cast<unsigned char>(header_content[i]))) {
            header_content.erase(i, i + 1);
            --i;
        }
    }
    return header_content;
}

void    rev_request_hosts(HTTPrequest &obj, std::stringstream &ss)
{
    std::string		header;
    size_t			iterator = 0;

    while (std::getline(ss, header)) {
        if (header.empty() || header == "\r")
            break ;
        iterator = header.find(":");
        if (iterator != std::string::npos) {
            std::string header_content = header.substr(iterator + 1);
            obj.headers.insert(std::make_pair(
                canonicalHeaderName(header.substr(0, iterator)),
                str_trim(header_content)));
        }
    }
    rev_request_body(obj, ss);
}

HTTPrequest fill_HTTP_object(std::stringstream &ss) {
	HTTPrequest obj;

	rev_request_firstLine(obj, ss);
	rev_request_hosts(obj, ss);

	return obj;
}