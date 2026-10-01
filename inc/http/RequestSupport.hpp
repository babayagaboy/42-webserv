#ifndef REQUEST_SUPPORT_HPP
# define REQUEST_SUPPORT_HPP

# include <string>
# include <vector>

class Client;
class HTTPrequest;
class HTTPresponse;
class Location;
class Server;

std::string convertToUpperCase(std::string text);
std::string buildEnvVariavle(const std::string &name, const std::string &value);
std::vector<std::string> buildEnvironment(const Client &client, const Server &server, std::string scriptPath);
int sendCGIResponse(Client &client, Server &server, const std::string &cgiResponse);
std::string buildFilePath(const Location &location, const std::string &requestPath);
int getFilesFolder(Client &client, Server &server, HTTPresponse &response, const std::string &path);
int checkIPaddress(std::string ip);
std::string findCGIcompiler(const std::string &extension);
void print_info(const HTTPrequest &request);

#endif