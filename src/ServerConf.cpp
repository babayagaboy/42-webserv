/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 13:12:54 by myivanov          #+#    #+#             */
/*   Updated: 2026/08/05 16:42:34 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConf.hpp"

ServerConf::ServerConf() : _listenPort(0), _serverName(""), _host(""), _clientMaxSize(0) {}

ServerConf::ServerConf(const ServerConf &obj) : _listenPort(obj._listenPort), _serverName(obj._serverName), 
												_host(obj._host), _clientMaxSize(obj._clientMaxSize), _objLocs(obj._objLocs) {}

ServerConf& ServerConf::operator=(const ServerConf &obj) {
	if (this != &obj) {
		_listenPort = obj._listenPort;
		_serverName = obj._serverName;
		_host = obj._host;
		_clientMaxSize = obj._clientMaxSize;
		_objLocs = obj._objLocs;
	}
	return *this;
}

ServerConf::~ServerConf() {}

void	ServerConf::setListenPort(unsigned int num){ _listenPort = num; }

void	ServerConf::setServerName(const std::string &text) { _serverName = text; }

void	ServerConf::setHost(const std::string &text) { _host = text; }

void	ServerConf::setClientMaxSize(size_t num) { _clientMaxSize = num; }

void	ServerConf::setObjLocs(const Location &obj)
{
	_objLocs.push_back(obj);
}


unsigned int ServerConf::getListenPort() const { return _listenPort; }

std::string ServerConf::getServerName() const { return _serverName; }

std::string ServerConf::getHost() const { return _host; }

size_t	ServerConf::getClientMaxSize() const { return _clientMaxSize; }

const std::vector<Location>& ServerConf::getLocations() const { return _objLocs; }

std::ostream&   operator<<(std::ostream &stream, const ServerConf &obj)
{
    std::vector<Location> locVec = obj.getLocations();

    stream << "ServerConf ListenPort: " << obj.getListenPort() << std::endl;
    stream << "ServerConf ServerName: " << obj.getServerName() << std::endl;
    stream << "ServerConf Host: " << obj.getHost() << std::endl;
    stream << "ServerConf ClientMaxSize: " << obj.getClientMaxSize() << std::endl << std::endl;
    stream << "ServerConf ObjLocs: " << std::endl;
    for (size_t i = 0; i < locVec.size(); ++i)
    {
        stream << locVec[i] << std::endl;
    }

    return stream;
}