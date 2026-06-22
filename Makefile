NAME	= ircserv
CXX		= c++
CXXFLAGS= -Wall -Wextra -Werror -std=c++98 -Iinclude -Itests
SRC		= main.cpp srcs/Server.cpp srcs/Reply.cpp \
		  srcs/Channels/Channel.cpp srcs/Channels/Channels.cpp \
		  srcs/Clients/Client.cpp srcs/Clients/Clients.cpp \
		  srcs/Command/Command.cpp srcs/Command/CmdTypes.cpp \
		  srcs/Command/handlers/auth.cpp srcs/Command/handlers/channel.cpp \
		  srcs/Command/handlers/messaging.cpp srcs/Command/handlers/misc.cpp
OBJ		= $(SRC:.cpp=.o)

TEST_NAME	= run_tests
TEST_SRC	= tests/test_main.cpp tests/unit/test_cmdtypes.cpp tests/unit/test_command.cpp \
			  tests/unit/test_channels.cpp tests/unit/test_clients.cpp \
			  tests/unit/test_reply.cpp \
			  tests/unit/test_handlers_auth.cpp tests/unit/test_handlers_channel.cpp \
			  tests/unit/test_handlers_messaging.cpp tests/unit/test_handlers_misc.cpp \
			  tests/unit/test_access_control.cpp \
			  tests/unit/test_session.cpp
TEST_OBJ	= $(TEST_SRC:.cpp=.o)
TEST_LIB	= srcs/Server.cpp srcs/Reply.cpp srcs/Command/CmdTypes.cpp srcs/Command/Command.cpp \
			  srcs/Command/handlers/auth.cpp srcs/Command/handlers/channel.cpp \
			  srcs/Command/handlers/messaging.cpp srcs/Command/handlers/misc.cpp \
			  srcs/Channels/Channel.cpp srcs/Channels/Channels.cpp \
			  srcs/Clients/Client.cpp srcs/Clients/Clients.cpp
TEST_LIBOBJ	= $(TEST_LIB:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)
	echo "$(NAME) generated"

test: $(TEST_NAME)

$(TEST_NAME): $(TEST_OBJ) $(TEST_LIBOBJ)
	$(CXX) $(CXXFLAGS) -o $(TEST_NAME) $(TEST_OBJ) $(TEST_LIBOBJ)
	echo "$(TEST_NAME) generated"

test_run: test
	./$(TEST_NAME)

integration_test:
	python3 scripts/integration_test.py

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TEST_OBJ) $(TEST_LIBOBJ) $(TEST_NAME)
	echo "object files deleted"

fclean: clean
	rm -f $(NAME)
	echo "$(NAME) deleted"

re: fclean all

.PHONY: all clean fclean re test test_run integration_test
.SILENT:
