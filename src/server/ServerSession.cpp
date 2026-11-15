#include <Server.hpp>
#include <cctype>
#include <sstream>

std::string Server::createSession() 
{
	std::stringstream ss;
	std::stringstream sss;

	++sessionCounter;
	ss << sessionCounter;
	sss << (serverid + 1);

	std::string serverId = sss.str();
	std::string id = "S" + serverId + "-" + ss.str();

	Session session;
	session.id = id;

	sessions[id] = session;

	return id;
}

std::string Server::getSessionId(const Client &c) const
{
    std::map<std::string, std::string>::const_iterator it;

    it = c.request.headers.find("Cookie");
    if (it == c.request.headers.end())
        return "";

    std::string cookie = it->second;

    size_t pos = 0;
    while (pos < cookie.size())
    {
        size_t semi = cookie.find(';', pos);
        size_t pairEnd = (semi == std::string::npos) ? cookie.size() : semi;

        std::string pair = cookie.substr(pos, pairEnd - pos);
        size_t start = 0;
        while (start < pair.size() && isspace(static_cast<unsigned char>(pair[start]))) ++start;
        size_t end = pair.size();
        while (end > start && isspace(static_cast<unsigned char>(pair[end - 1]))) --end;

        if (end > start)
        {
            std::string trimmed = pair.substr(start, end - start);
            size_t eq = trimmed.find('=');
            if (eq != std::string::npos)
            {
                std::string key = trimmed.substr(0, eq);
                std::string val = trimmed.substr(eq + 1);

                for (size_t i = 0; i < key.size(); ++i)
                    key[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(key[i])));

                if (key == "sessionid")
                    return val;
            }
        }

        if (semi == std::string::npos)
            break;
        pos = semi + 1;
    }

    return "";
}

bool Server::hasSession(const Client &c) const
{
    std::string id = getSessionId(c);

    if (id.empty())
        return false;

    return sessions.find(id) != sessions.end();
}

void Server::handleSession(Client &c)
{
    std::string id = getSessionId(c);

   // std::map<std::string, std::string>::const_iterator cookie =
     //   c.request.headers.find("Cookie");

    if (!id.empty())
    {
        std::map<std::string, Session>::iterator it = sessions.find(id);

        if (it != sessions.end())
        {
            c.sessionId = id;
            c.newSession = false;

            ++it->second.requestCount;

            return;
        }
    }

    c.sessionId = createSession();
    sessions[c.sessionId].requestCount = 1;
    c.newSession = true;
}

Session::Session() : isLoggedIn(false), requestCount(0) {}
