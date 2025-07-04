#pragma once

#include <sstream>
#include "Error.hpp"

#ifndef DEBUG
    #define DEBUG 0
#endif

#define IRC_NAME "burekserv"
#define DATA_MAX 512

#define ANSI_RED "\033[0;31m"
#define ANSI_GREEN "\033[0;32m"
#define ANSI_YELLOW "\033[0;33m"
#define ANSI_PINK "\033[0;35m"
#define ANSI_CYAN "\033[0;36m"
#define ANSI_WHITE "\033[0:36m"
#define ANSI_RESET "\033[0m"

namespace utils {

bool is_space(char c);
bool is_alpha(char c);
bool is_digit(char c);
bool is_channel(std::string s);
bool is_alphanumeric(char c);
bool is_underscore(char c);
bool is_invalid(char c);
bool is_valid_first_nickname( char c);
bool is_valid_nickname( char c);
bool is_nickname(std::string s);
bool is_username(std::string s);
template< typename T >
T str_to_value(std::string str) {
    T value;
    std::istringstream(str) >> value;
    return value;
}

} // namespace utils
