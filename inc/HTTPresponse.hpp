/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPresponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 16:12:13 by myivanov          #+#    #+#             */
/*   Updated: 2026/08/16 17:02:01 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include <iostream>
# include <vector>
# include <sstream>

class HTTPresponse {
		int                                                 _statusCode;
		std::vector<std::pair<std::string, std::string> >   _headers;
		std::string                 _body;
	public:
		HTTPresponse();
		HTTPresponse(const HTTPresponse &obj);
		HTTPresponse& operator=(const HTTPresponse &obj);
		~HTTPresponse();

		int                                 					getStatusCode();
		const std::vector<std::pair<std::string, std::string> >&	getHeaders() const;
		const std::string&											getBody() const;

		void                                						setStatusCode(int code);
		void														setHeaders(const std::vector<std::pair<std::string, std::string> > &head);
		void														setBody(const std::string &body);

		std::string														buildResponse() const;
};

#endif