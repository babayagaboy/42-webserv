/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: myivanov <myivanov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 18:00:29 by hgutterr          #+#    #+#             */
/*   Updated: 2026/09/16 16:49:41 by myivanov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>
# include <vector>
# include "Client.hpp"


class Location {
	private:
		std::string		_path;
		std::string		_defaultRoot;
		std::string		_index;
		std::string		_allowedMethods[9];
		std::vector<std::pair<int, std::string> > _errorPage;
		std::vector<std::pair<int, std::string> >_return;
		std::vector<std::pair<std::string, std::string> >_cgi;
		bool			_autoIndex;
		size_t			_clientMaxSize;
		
		int				_counter;
	public:
		Location();
		Location(const Location &obj);
		Location& operator=(const Location &obj);
		~Location();

		void	setPath(const std::string &text);
		void	setDefaultRoot(const std::string &text);
		void	setIndex(const std::string &text);
		void	setAllowedMethods(const std::string &text);
		void	setAutoIndex(const std::string &text);
		void	setClientMaxSize(size_t size);
		void	setErrorPage(int numErr, const std::string &text);
		void	setReturn(int ret, const std::string &text);
		void	setCgi(const std::string &exec, const std::string &path);

		std::string		getPath() const;
		std::string		getDefaultRoot() const;
		std::string		getIndex() const;
		const std::string*	getAllowedMethods() const;
		bool			getAutoIndex() const;
		size_t		getClientMaxSize() const;
		const std::vector<std::pair<int, std::string> >& getErrorPage() const;
		const std::vector<std::pair<int, std::string> >& getReturn() const;
		const std::vector<std::pair<std::string, std::string> >& getCgi() const;

		std::string getPagePath() const;
};


std::ostream&   operator<<(std::ostream &stream, const Location &obj);


#endif