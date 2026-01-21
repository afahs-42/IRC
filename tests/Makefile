# Test Makefile

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# Source files
PARSER_TEST_SRCS = test_parser.cpp ../srcs/Parser.cpp ../srcs/Command.cpp
PARSER_TEST_OBJS = $(PARSER_TEST_SRCS:.cpp=.o)

# Include directories
INCLUDES = -I ../includes

# Test binaries
PARSER_TEST = test_parser

all: $(PARSER_TEST)

$(PARSER_TEST): $(PARSER_TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(PARSER_TEST_OBJS) -o $(PARSER_TEST)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(PARSER_TEST_OBJS)
	rm -f ../srcs/Parser.o ../srcs/Command.o

fclean: clean
	rm -f $(PARSER_TEST)

re: fclean all

run: all
	@echo "Running parser unit tests..."
	@./$(PARSER_TEST)

.PHONY: all clean fclean re run
