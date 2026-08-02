/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverConf.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 17:39:16 by hgutterr          #+#    #+#             */
/*   Updated: 2026/08/02 11:49:12 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGFILE_HPP
# define CONFIGFILE_HPP

#include <location.hpp>
#include <iostream>
#include <list>

class serverConf {
	private:
		//unsigned int		_listenPort;
		std::string			_serverName;
		std::string			_host;
		//size_t				_clientMaxSize;

		std::list<location>	_objLocs;
};

#endif