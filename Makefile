# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/07/29 15:00:33 by hgutterr          #+#    #+#              #
#    Updated: 2026/10/01 22:26:45 by hgutterr         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		= webserv

CXX			= c++
CXXFLAGS	= -Wall -Werror -Wextra -std=c++98
CXXFLAGS	+= -Iinc/config -Iinc/http -Iinc/server

SRCDIR		= src
SRCS		= $(addprefix $(SRCDIR)/, \
				app/main.cpp \
				config/configFile.cpp \
				config/keyWordsFunc.cpp \
				config/Location.cpp \
				config/ServerConf.cpp \
				http/HTTPrequest.cpp \
				http/HTTPresponse.cpp \
				http/fillHTTPobject.cpp \
				http/RequestHandlers.cpp \
				http/RequestDispatch.cpp \
				http/RequestSupport.cpp \
				server/Client.cpp \
				server/Server.cpp)

OBJDIR		= obj
OBJS		= $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "\nReady!"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

noflags:
	$(CXX) $(SRCS) -Iinc/config -Iinc/http -Iinc/server -o $(NAME)
	@echo "\nReady without flags!"

.PHONY: all clean fclean re