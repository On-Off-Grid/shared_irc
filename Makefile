# **************************************************************************** #
#                                                                              #
#    ft_irc - Phase 1 & 2: Project Skeleton & Data Processing Layer           #
#                                                                              #
#    Makefile for building the ircserv executable                               #
#                                                                              #
# **************************************************************************** #

NAME		= ircserv

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98

INCLUDES	= -Iincludes

SRC_DIR		= src
OBJ_DIR		= obj

SRCS		= src/main.cpp \
			  src/Server.cpp \
			  src/Client.cpp \
			  src/Channel.cpp \
			  src/Replies.cpp \
			  src/CommandDispatcher.cpp \
			  src/BufferManager.cpp \
			  src/Parser.cpp

OBJS		= $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# **************************************************************************** #
#                              RULES                                           #
# **************************************************************************** #

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "✅ $(NAME) compiled successfully."

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
	@echo "🧹 Object files removed."

fclean: clean
	rm -f $(NAME)
	@echo "🗑️  $(NAME) removed."

re: fclean all

.PHONY: all clean fclean re
