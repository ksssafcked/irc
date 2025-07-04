#include "Common.hpp"

namespace utils {

bool is_space(char c) { return c == ' ' || c == '\t'; }

bool is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

bool is_digit(char c) { return c >= '0' && c <= '9'; }

bool is_alphanumeric(char c) { return is_alpha(c) || is_digit(c); }

bool is_underscore(char c) { return c == '-' || c == '_';}

bool is_valid_first_nickname( char c) { return is_alphanumeric(c) || c == '-' || c == '[' || c ==']' || c == '\\';}

bool is_valid_nickname( char c) { return is_alpha(c) || c == '[' || c ==']' || c == '\\';}

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

bool is_nickname(std::string s) {
    if (s.size() < 1 || s.size() > 9)
        return false;
    if (!is_valid_first_nickname(s[0]))
        return false;
    for (size_t i = 1; i < s.size(); i++) {
        if (!is_valid_nickname(s[i]))
            return false;
    }
    return true;
}

bool is_username(std::string s) {
    if (s.find("@") != std::string::npos) {
        return false;
    }
    if (s.find("\r") != std::string::npos) {
        return false;
    }
    if (s.find("\n") != std::string::npos) {
        return false;
    }
    return true;
}

} // namespace utils
