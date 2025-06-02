#include <string>
#include "Common.hpp"

std::string error_username_inuse(std::string username) {
	return (std::string(":" + std::string(IRC_NAME) + " 433 * " + username + " : is already connected\r\n"));
}

std::string error_nickname_inuse(std::string username) {
	return (std::string(":" + std::string(IRC_NAME) + " 433 * " + username + " :nickname is already in use\r\n"));
}