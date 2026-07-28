# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/07/27 20:07:11 by hgutterr          #+#    #+#              #
#    Updated: 2026/07/27 20:07:11 by hgutterr         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		= webserv

CXX 		= c++
CXXFLAGS 	= -Wall -Werror -Wextra -std=c++98

SRCS 		= main.cpp \
			configFile.cpp \
			fill_HTTP_object.cpp \
			HTTPrequest.cpp \
			server.cpp


OBJDIR		= obj
OBJS		= $(SRCS:%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "\nReady!"

$(OBJDIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

noflags:
	$(CXX) $(SRCS) -o $(NAME)
	@echo "\nReady without flags!"

.PHONY: all clean fclean re
