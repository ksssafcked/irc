#include "Common.hpp"

namespace utils {

bool is_space(char c) { return c == ' ' || c == '\t'; }

bool is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

bool is_digit(char c) { return c >= '0' && c <= '9'; }

bool is_alphanumeric(char c) { return is_alpha(c) || is_digit(c); }

} // namespace utils
