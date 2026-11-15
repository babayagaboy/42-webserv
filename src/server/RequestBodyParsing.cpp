/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestBodyParsing.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/15 00:10:45 by hgutterr          #+#    #+#             */
/*   Updated: 2026/11/15 00:10:54 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstddef>
#include <sstream>
#include <string>

int parseContentLength(const std::string &value, size_t &length)
{
	std::stringstream stream(value);
	char extra;

	if (!(stream >> length) || (stream >> extra))
		return 0;
	return 1;
}

int decodeChunkedBody(const std::string &buffer, size_t bodyStart,
								 std::string &body, size_t &consumed,
								size_t maxBodySize)
{
	size_t position = bodyStart;
	body.clear();

	while (true)
	{
		size_t lineEnd = buffer.find("\r\n", position);
		if (lineEnd == std::string::npos)
			return 0;

		std::string sizeText = buffer.substr(position, lineEnd - position);
		size_t extension = sizeText.find(';');
		if (extension != std::string::npos)
			sizeText.erase(extension);
		if (sizeText.empty())
			return -1;

		unsigned long chunkSize = 0;
		std::stringstream sizeStream;
		sizeStream << std::hex << sizeText;
		if (!(sizeStream >> chunkSize))
			return -1;

		position = lineEnd + 2;
		if (chunkSize == 0)
		{
			size_t trailers = buffer.find("\r\n\r\n", position);
			if (trailers == std::string::npos)
			{
				if (buffer.compare(position, 2, "\r\n") == 0)
				{
					consumed = position + 2;
					return 1;
				}
				return 0;
			}
			consumed = trailers + 4;
			return 1;
		}

		if (position > buffer.size() || chunkSize > buffer.size() - position
			|| buffer.size() - position - chunkSize < 2)
			return 0;
		if (maxBodySize != 0 && chunkSize > maxBodySize - body.size())
			return -2;
		body.append(buffer, position, chunkSize);
		position += chunkSize;
		if (buffer.compare(position, 2, "\r\n") != 0)
			return -1;
		position += 2;
	}
}

