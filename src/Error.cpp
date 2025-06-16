#include <string>
#include "Common.hpp"

std::string error_username_inuse(std::string username) {
	return (std::string(":" + std::string(IRC_NAME) + " 433 * " + username + " : is already connected\r\n"));
}

std::string error_nickname_inuse(std::string username) {
	return (std::string(":" + std::string(IRC_NAME) + " 433 * " + username + " :nickname is already in use\r\n"));
}

std::string error_nosuchchannel(std::string username, std::string channel) {
	return (std::string(":" + std::string(IRC_NAME) + " 403 * " + username " " + channel + " :No such channel\r\n"));
}

std::string error_cannotsendtochan(std::string username, std::string channel) {
	return (std::string(":" + std::string(IRC_NAME) + " 404 * " + username + " " + channel + " :Cannot send to channel\r\n"));
}

// std::string error_nosuchchannel(std::string username, std::string channel) {
// 	return (std::string(":" + std::string(IRC_NAME) + " 433 * " + username + " :nickname is already in use\r\n"));
// }

// std::string error_nickname_inuse(std::string username) {
// 	return (std::string(":" + std::string(IRC_NAME) + " 433 * " + username + " :nickname is already in use\r\n"));
// }

// std::string error_nickname_inuse(std::string username) {
// 	return (std::string(":" + std::string(IRC_NAME) + " 433 * " + username + " :nickname is already in use\r\n"));
// }

// std::string error_nickname_inuse(std::string username) {
// 	return (std::string(":" + std::string(IRC_NAME) + " 433 * " + username + " :nickname is already in use\r\n"));
// }

// std::string error_nickname_inuse(std::string username) {
// 	return (std::string(":" + std::string(IRC_NAME) + " 433 * " + username + " :nickname is already in use\r\n"));
// }

// std::string error_nickname_inuse(std::string username) {
// 	return (std::string(":" + std::string(IRC_NAME) + " 433 * " + username + " :nickname is already in use\r\n"));
// }