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

SRCS := src/main.cpp src/Server.cpp src/Client.cpp src/Common.cpp

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
	@./build/bin/ircserv 6667 42

re: fclean all

.PHONY: all clean fclean re
