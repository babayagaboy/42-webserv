/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configFile.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 17:39:16 by hgutterr          #+#    #+#             */
/*   Updated: 2026/07/27 18:13:05 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGFILE_HPP
# define CONFIGFILE_HPP

#include <iostream>

class configFile {
	private:
		unsigned int	_listenPort;
		std::string		_serverName;
		std::string		_host;
		size_t			_clientMaxSize;

		// location
		std::string		_defaultRoot;
		std::string		_index;
		std::string		_allowedMethodsNames[9];
		int				_allowedMethods[9];
};

#endif