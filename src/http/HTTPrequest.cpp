/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPrequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 15:17:34 by myivanov          #+#    #+#             */
/*   Updated: 2026/07/29 15:16:11 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <HTTPrequest.hpp>

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
