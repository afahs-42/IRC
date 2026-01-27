NAME = ircserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = srcs/main.cpp \
       srcs/Server.cpp \
       srcs/Client.cpp \
       srcs/Parser.cpp \
       srcs/Channel.cpp \
       srcs/Command.cpp \
       srcs/utils/Utils.cpp \
       srcs/commands/Pass.cpp \
       srcs/commands/Nick.cpp \
       srcs/commands/User.cpp \
       srcs/commands/Join.cpp \
       srcs/commands/Privmsg.cpp

OBJ_DIR = obj
OBJS = $(SRCS:srcs/%.cpp=$(OBJ_DIR)/%.o)

INCLUDES = -I includes

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: srcs/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re