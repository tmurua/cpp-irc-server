NAME	= ircserv
CXX		= c++
CXXFLAGS= -Wall -Wextra -Werror -std=c++98 -Iinclude -Itests
SRC		= main.cpp srcs/Server.cpp
OBJ		= $(SRC:.cpp=.o)

TEST_NAME	= run_tests
TEST_SRC	= tests/test_main.cpp tests/unit/test_cmdtypes.cpp tests/unit/test_command.cpp
TEST_OBJ	= $(TEST_SRC:.cpp=.o)
TEST_LIB	= srcs/Command/CmdTypes.cpp srcs/Command/Command.cpp
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

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TEST_OBJ) $(TEST_LIBOBJ) $(TEST_NAME)
	echo "object files deleted"

fclean: clean
	rm -f $(NAME)
	echo "$(NAME) deleted"

re: fclean all

.PHONY: all clean fclean re test test_run
.SILENT:
