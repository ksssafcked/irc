#pragma once

#include <sstream>

#ifndef DEBUG
    #define DEBUG 0
#endif

namespace utils {

template< typename T >
T str_to_value(std::string str) {
    T value;
    std::istringstream(str) >> value;
    return value;
}

} // namespace utils
