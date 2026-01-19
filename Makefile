NAME = parser_test

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = srcs/Parser.cpp \
	   srcs/Command.cpp

OBJS = $(SRCS:.cpp=.o)

INCLUDES = includes/Parser.hpp includes/Command.hpp

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp $(INCLUDES)
	$(CXX) $(CXXFLAGS) -I includes -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
