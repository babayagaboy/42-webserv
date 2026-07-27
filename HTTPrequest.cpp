/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPrequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 15:17:34 by myivanov          #+#    #+#             */
/*   Updated: 2026/07/27 15:18:32 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPrequest.hpp"

HTTPrequest::HTTPrequest() {}

HTTPrequest::HTTPrequest(const HTTPrequest &obj) : method(obj.method), path(obj.path), version(obj.version),
                                                   headers(obj.headers), body(obj.body){}

HTTPrequest& HTTPrequest::operator=(const HTTPrequest &obj) {
    if (this != &obj) {
        method = obj.method;
        path = obj.path;
        version = obj.version;
        headers = obj.headers;
        body = obj.body;
    }
    return *this;
}

HTTPrequest::~HTTPrequest() {}
