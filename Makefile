NAME	= ircserv
CXX		= c++
CXXFLAGS= -Wall -Wextra -Werror -std=c++98 -Iinclude
SRC		= main.cpp srcs/Server.cpp srcs/Reply.cpp \
		  srcs/Channels/Channel.cpp srcs/Channels/Channels.cpp \
		  srcs/Clients/Client.cpp srcs/Clients/Clients.cpp \
		  srcs/Command/Command.cpp srcs/Command/CmdTypes.cpp \
		  srcs/Command/handlers/auth.cpp srcs/Command/handlers/channel.cpp \
		  srcs/Command/handlers/messaging.cpp srcs/Command/handlers/misc.cpp
OBJ		= $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)
	echo "$(NAME) generated"

integration_test: $(NAME)
	python3 scripts/integration_test.py

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)
	echo "object files deleted"

fclean: clean
	rm -f $(NAME)
	echo "$(NAME) deleted"

re: fclean all

.PHONY: all clean fclean re integration_test
.SILENT:
