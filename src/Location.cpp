/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 14:44:10 by myivanov          #+#    #+#             */
/*   Updated: 2026/08/21 20:51:16 by hgutterr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

Location::Location() : _path(""), _defaultRoot(""), _index(""), _autoIndex(false), _counter(0) {}

Location::Location(const Location &obj) : _path(obj._path), _defaultRoot(obj._defaultRoot), _index(obj._index),
											 _autoIndex(obj._autoIndex), _counter(obj._counter)
{
	for (int i = 0; i < 9; ++i){
			_allowedMethods[i] = obj._allowedMethods[i];
		}

    _errorPage = obj._errorPage;
    _return = obj._return;
    _cgi = obj._cgi;
}

Location& Location::operator=(const Location &obj) {
	if (this != &obj) {
		_path = obj._path;
		_defaultRoot = obj._defaultRoot;
		_index = obj._index;
		for (int i = 0; i < 9; ++i){
			_allowedMethods[i] = obj._allowedMethods[i];
		}
		_autoIndex = obj._autoIndex;
		_counter = obj._counter;
        _errorPage = obj._errorPage;
        _return = obj._return;
        _cgi = obj._cgi;
        
	}
	return *this;
}


Location::~Location() {}


void	Location::setPath(const std::string &text) { _path = text; }

void	Location::setDefaultRoot(const std::string &text) { _defaultRoot = text; }

void	Location::setIndex(const std::string &text) { _index = text; }

void	Location::setAllowedMethods(const std::string &text)
{
	if (_counter > 8)
		return ;
	
	_allowedMethods[_counter++] = text; 
}

void	Location::setAutoIndex(const std::string &text) {
	if (text == "true")
		_autoIndex = true;
	else
		_autoIndex = false;
}

void	Location::setErrorPage(int numErr, const std::string &text)
{
	_errorPage.push_back(std::make_pair(numErr, text));
}

void	Location::setReturn(int ret, const std::string &text)
{
	_return.push_back(std::make_pair(ret, text));
}

void	Location::setCgi(const std::string &exec, const std::string &path)
{
	_cgi.push_back(std::make_pair(exec, path));
}

std::string		Location::getPath() const { return _path; }

std::string		Location::getDefaultRoot() const { return _defaultRoot; }

std::string		Location::getIndex() const { return _index; }

const std::string*	Location::getAllowedMethods() const { return _allowedMethods; }

bool			Location::getAutoIndex() const { return _autoIndex; }

const std::vector<std::pair<int, std::string> >& Location::getErrorPage() const
{
	return _errorPage;
}

const std::vector<std::pair<int, std::string> >& Location::getReturn() const
{
	return _return;
}

const std::vector<std::pair<std::string, std::string> >& Location::getCgi() const
{
	return _cgi;
}


std::ostream&   operator<<(std::ostream &stream, const Location &obj)
{
    const std::string *methodsstr = obj.getAllowedMethods();
    std::vector<std::pair<int, std::string> > err = obj.getErrorPage();
    std::vector<std::pair<int, std::string> > ret = obj.getReturn();
    std::vector<std::pair<std::string, std::string> > cgi = obj.getCgi();

    stream << "Location path: " << obj.getPath() << std::endl;
    stream << "Location default root: " << obj.getDefaultRoot() << std::endl;
    stream << "Location index: " << obj.getIndex() << std::endl;
    stream << "Location Allowed Methods: ";
    for (int i = 0; i < 9; ++i) {
        if (methodsstr[i].empty())
            break ;
        stream << methodsstr[i] << " ";
    }
    stream << std::endl;
    for (size_t i = 0; i < err.size(); ++i)
    {
        stream << "Location error_page[" << i + 1 <<"]: ";
        stream << err[i].first << " " << err[i].second << std::endl;
    }

    for (size_t i = 0; i < ret.size(); ++i)
    {
        stream << "Location _return: ";
        stream << ret[i].first << " " << ret[i].second << std::endl;
    }

    for (size_t i = 0; i < cgi.size(); ++i)
    {
        stream << "Location _cgi[" << i + 1 << "]: ";
        stream << cgi[i].first << " " << cgi[i].second << std::endl;
    }
    stream << "Location autoIndex: " << obj.getAutoIndex();

    return stream;
}

std::string Location::getPagePath() const
{
    std::string root = _defaultRoot;
    std::string index = _index;

	if(index.empty())
		return ("." + root);

    if (!root.empty() && root[0] == '/')
        root = "." + root;

    if (!root.empty() && root[root.size() - 1] != '/')
        root += '/';

    while (!index.empty() && index[0] == '/')
        index.erase(0, 1);

    return root + index;
}