/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 17:39:16 by hgutterr          #+#    #+#             */
/*   Updated: 2026/08/05 16:41:41 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGFILE_HPP
# define CONFIGFILE_HPP

#include <Location.hpp>
#include <iostream>
#include <vector>

class ServerConf {
	private:
		unsigned int		_listenPort;
		std::string			_serverName;
		std::string			_host;
		size_t				_clientMaxSize;

		std::vector<Location>	_objLocs;
	public:
		ServerConf();
		ServerConf(const ServerConf &obj);
		ServerConf &operator=(const ServerConf &obj);
		~ServerConf();

		void	setListenPort(unsigned int num);
		void	setServerName(const std::string &text);
		void	setHost(const std::string &text);
		void	setClientMaxSize(size_t num);
		void	setObjLocs(const Location &obj);

		unsigned int	getListenPort() const;
		std::string		getServerName() const;
		std::string		getHost()		const;
		size_t			getClientMaxSize() const;
		const std::vector<Location>& getLocations() const;
};

std::ostream&   operator<<(std::ostream &stream, const ServerConf &obj);

#endif