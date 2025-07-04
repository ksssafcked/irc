#include "Common.hpp"

namespace utils {

bool is_space(char c) { return c == ' ' || c == '\t'; }

bool is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

bool is_digit(char c) { return c >= '0' && c <= '9'; }

bool is_alphanumeric(char c) { return is_alpha(c) || is_digit(c); }

bool is_underscore(char c) { return c == '-' || c == '_';}

bool is_invalid(char c) { return c == ',' || c == ' ' || c == '\x07' || c == ':';}

bool is_channel(std::string s) {
    if (s.size() <= 1 || s.size() > 50)
        return false;
    if (s[0] != '#')
        return false;
    for (size_t i = 1; i < s.size(); i++){
        if (is_invalid(s[i]))
            return false;
    }
    return true;
}

} // namespace utils
