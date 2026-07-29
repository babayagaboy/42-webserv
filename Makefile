# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hgutterr <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/07/29 15:00:33 by hgutterr          #+#    #+#              #
#    Updated: 2026/07/29 15:15:44 by hgutterr         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


NAME        = webserv

CXX         = c++
CXXFLAGS    = -Wall -Werror -Wextra -std=c++98
CXXFLAGS    += -Iinc

SRCDIR      = src
SRCS        = $(addprefix $(SRCDIR)/, \
                main.cpp \
				Client.cpp \
				configFile.cpp \
				fillHTTPobject.cpp \
				HTTPrequest.cpp \
				Server.cpp
              )

OBJDIR      = obj
OBJS        = $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

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
	$(CXX) $(SRCS) -Iinc -o $(NAME)
	@echo "\nReady without flags!"

.PHONY: all clean fclean re