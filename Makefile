.DEFAULT_GOAL := all

# ANSI
ANSI_GREEN := \033[1;32m
ANSI_GREY := \033[0;37m
ANSI_RESET := \033[0m

# Compiler
CXX := c++

CXXFLAGS = -std=c++98
CXXFLAGS += -Wall -Wextra -Werror
CXXFLAGS += -g

# Project
PROJ_NAME := ircserv
BUILD_DIR := ./build
BIN_DIR := $(BUILD_DIR)/bin
OBJ_DIR := $(BUILD_DIR)/obj

NAME := $(BIN_DIR)/$(PROJ_NAME)

SRCS := src/main.cpp src/Server.cpp src/Client.cpp src/Common.cpp src/Commands.cpp src/Channel.cpp src/Error.cpp

OBJS := $(SRCS:src/%.cpp=$(OBJ_DIR)/%.o)

# Rules
all: $(NAME)

debug: CXXFLAGS += -DDEBUG=1
debug: $(NAME)

$(NAME): $(OBJS)
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(OBJS) $(CXXFLAGS) -o $@
	@echo -e "$(ANSI_GREEN)[make][$(PROJ_NAME)]:$(ANSI_RESET) built $(ANSI_GREEN)bin$(ANSI_RESET) at $(NAME)"

$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo -e "$(ANSI_GREEN)[make][$(PROJ_NAME)]:$(ANSI_RESET) cleaning $(ANSI_GREEN)obj$(ANSI_RESET) files"
	@rm -rf $(OBJ_DIR)

fclean:
	@echo -e "$(ANSI_GREEN)[make][$(PROJ_NAME)]:$(ANSI_RESET) cleaning $(ANSI_GREEN)build$(ANSI_RESET) files"
	@rm -rf $(BUILD_DIR)

test: re
	@./build/bin/ircserv 8080 4242

tests: re
	@python3 ./test/test.py

irssi:
	@docker build . -t irssi
	@docker run -it --rm --network host irssi bash

a:
	irssi --connect=localhost -p 8080 -w 4242 --home=./test/user_a

b:
	irssi --connect=localhost -p 8080 -w 4242 --home=./test/user_b

re: fclean all

.PHONY: all clean fclean re
