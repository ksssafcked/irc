#pragma once

#include <sstream>

#ifndef DEBUG
    #define DEBUG 0
#endif

#define ANSI_YELLOW "\033[0;33m"
#define ANSI_RED "\033[0;31m"
#define ANSI_GREEN "\033[0;32m"
#define ANSI_CYAN "\033[0;36m"
#define ANSI_WHITE "\033[0:36m"
#define ANSI_RESET "\033[0m"

namespace utils {

template< typename T >
T str_to_value(std::string str) {
    T value;
    std::istringstream(str) >> value;
    return value;
}

} // namespace utils
