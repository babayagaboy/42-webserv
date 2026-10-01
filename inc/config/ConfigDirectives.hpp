#ifndef CONFIG_DIRECTIVES_HPP
# define CONFIG_DIRECTIVES_HPP

# include <Server.hpp>

int handle_listen(std::vector<std::string> &tokens, size_t index, Counter &counter);
int handle_host(std::vector<std::string> &tokens, size_t index, Counter &counter);
int handle_server_name(std::vector<std::string> &tokens, size_t index, Counter &counter);
int handle_client_max_size(std::vector<std::string> &tokens, size_t index, Counter &counter);
int handle_location_client_max_size(std::vector<std::string> &tokens, size_t &index, CounterLocation &counter);
int handle_root(std::vector<std::string> &tokens, size_t &index, CounterLocation &counter);
int handle_index(std::vector<std::string> &tokens, size_t &index, CounterLocation &counter);
int handle_autoindex(std::vector<std::string> &tokens, size_t &index, CounterLocation &counter);
int handle_cgi(const std::vector<std::string> &tokens, const std::string keywords[], size_t &index);
int handle_return(const std::vector<std::string> &tokens, const std::string keywords[], size_t &index, CounterLocation &counter);
int handle_error_page(const std::vector<std::string> &tokens, const std::string keywords[], size_t &index);
int handle_allowed(const std::vector<std::string> &tokens, size_t &index, CounterLocation &counter);
bool checkExtension(const std::string &extension);
int checkValueisKeyword(const std::string &token, const std::string keywords[], const std::string &directive);

#endif