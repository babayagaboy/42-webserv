/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configFile.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 17:39:16 by hgutterr          #+#    #+#             */
/*   Updated: 2026/07/28 21:57:41 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGFILE_HPP
# define CONFIGFILE_HPP

#include "location.hpp"
#include <iostream>
#include <list>

class configFile {
	private:
		unsigned int		_listenPort;
		std::string			_serverName;
		std::string			_host;
		size_t				_clientMaxSize;
		std::list<location>	_objLocs;
};

#endif