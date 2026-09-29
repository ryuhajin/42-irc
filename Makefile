CC = c++

CFLAG = -Wall -Werror -Wextra -std=c++98 -MMD -MP -g3 -fsanitize=address

NAME = ircserv

SRCS =	main.cpp \
		server.cpp \
		parser.cpp \
		channel.cpp \
		client.cpp \
		utils.cpp \
		./cmd/cmd.cpp \
		./cmd/invite.cpp \
		./cmd/join.cpp \
		./cmd/kick.cpp \
		./cmd/mode.cpp \
		./cmd/nick.cpp \
		./cmd/part.cpp \
		./cmd/pass.cpp \
		./cmd/ping.cpp \
		./cmd/privmsg.cpp \
		./cmd/quit.cpp \
		./cmd/topic.cpp \
		./cmd/user.cpp

INCS = -I.

OBJS = $(SRCS:.cpp=.o)

DEPS = $(OBJS:%.o=%.d)

all : $(NAME)

$(NAME) : $(OBJS)
		$(CC) $(CFLAG) $(OBJS) -o $(NAME) 

%.o : %.cpp
	 $(CC) $(CFLAG) $(INCS) -c $< -o $@

clean : 
		rm -rf $(OBJS) $(DEPS)

fclean : clean 
	    rm -rf $(NAME)

re : fclean all

.PHONY : all clean fclean re

-include $(DEPS)
