#ifndef CONFIG_LOADER_HPP
# define CONFIG_LOADER_HPP

# include <vector>
# include <Server.hpp>

int fillServerConfig(char *configFileName, std::vector<Server> &servers);

#endif