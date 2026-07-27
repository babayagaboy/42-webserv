/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPrequest.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 15:15:49 by myivanov          #+#    #+#             */
/*   Updated: 2026/07/27 15:15:49 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <iostream>
# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <stdlib.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <map>
# include <sstream>

class HTTPrequest {
    public:
        std::string method;
        std::string path;
        std::string version;

        std::map<std::string, std::string> headers;

        std::string body;
        
        HTTPrequest();
        HTTPrequest(const HTTPrequest &obj);
        HTTPrequest& operator=(const HTTPrequest &obj);
        ~HTTPrequest();
};

#endif