/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 18:00:29 by hgutterr          #+#    #+#             */
/*   Updated: 2026/07/29 15:02:16 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>

class location {
	private:
		std::string		_name;
		std::string		_defaultRoot;
		std::string		_index;
		// std::string		_allowedMethodsNames[9];
		int				_allowedMethods[9];
		bool			_autoIndex;
};

#endif