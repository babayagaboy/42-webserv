#include <Server.hpp>

int Server::findLocation( const Client &c ) const
{
	std::string path = c.request.path;
	const std::vector<Location> &locations = serversConfs.getLocations();

	int bestLocation = -1;
	size_t bestLength = 0;

	if (!path.empty() && path[0] != '/')
		path = "/" + path;

	for (size_t i = 0; i < locations.size(); ++i)
	{
		const std::string &locationPath = locations[i].getPath();

		if (path.compare(0, locationPath.size(), locationPath) != 0)
			continue;

		if (locationPath != "/" &&
			locationPath[locationPath.size() - 1] != '/' &&
			path.size() != locationPath.size() &&
			path[locationPath.size()] != '/')
			continue;

		if (locationPath.size() > bestLength)
		{
			bestLength = locationPath.size();
			bestLocation = i;
		}
	}

	return bestLocation;
}

bool Server::isMethodAllowed( const std::string &method, int l ) const
{
	if (l < 0)
		return false;
	
	const std::string *allowedMethods = serversConfs.getLocations()[l].getAllowedMethods();
	for (size_t i = 0; i < 9; ++i)
	{
		if (allowedMethods[i].empty())
			break ;

		if (method == allowedMethods[i])
			return true;
	}
	return false;
}
