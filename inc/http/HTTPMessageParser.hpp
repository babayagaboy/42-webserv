#ifndef HTTP_MESSAGE_PARSER_HPP
# define HTTP_MESSAGE_PARSER_HPP

# include <sstream>
# include <HTTPrequest.hpp>

HTTPrequest fill_HTTP_object(std::stringstream &stream);

#endif