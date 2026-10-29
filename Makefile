.DEFAULT_GOAL := all

# ANSI
ANSI_GREEN := \033[1;32m
ANSI_RESET := \033[0m

# Compiler
CXX := c++

CXXFLAGS = -std=c++98
CXXFLAGS += -Wall -Wextra -Werror
CXXFLAGS += -g
CXXFLAGS += -MMD -MP

# Project
PROJ_NAME := ircserv
BUILD_DIR := ./build
BIN_DIR := $(BUILD_DIR)/bin
OBJ_DIR := $(BUILD_DIR)/obj

NAME := $(BIN_DIR)/$(PROJ_NAME)

SRCS := src/main.cpp src/Server.cpp src/Client.cpp src/Common.cpp src/Commands.cpp src/Channel.cpp src/Error.cpp

OBJS := $(SRCS:src/%.cpp=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# Rules
all: $(NAME)


$(NAME): $(OBJS)
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(OBJS) $(CXXFLAGS) -o $@
	@printf "$(ANSI_GREEN)[make][$(PROJ_NAME)]:$(ANSI_RESET) built $(ANSI_GREEN)bin$(ANSI_RESET) at $(NAME)\n"

$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@printf "$(ANSI_GREEN)[make][$(PROJ_NAME)]:$(ANSI_RESET) cleaning $(ANSI_GREEN)obj$(ANSI_RESET) files\n"
	@rm -rf $(OBJ_DIR)

fclean:
	@printf "$(ANSI_GREEN)[make][$(PROJ_NAME)]:$(ANSI_RESET) cleaning $(ANSI_GREEN)build$(ANSI_RESET) files\n"
	@rm -rf $(BUILD_DIR)

a:
	irssi --connect=localhost -p 8080 -w 4242 --home=./test/user_a

b:
	irssi --connect=localhost -p 8080 -w 4242 --home=./test/user_b

re: fclean all

-include $(DEPS)

.PHONY: all debug clean fclean re a b