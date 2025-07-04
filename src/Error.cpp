#include <string>
#include "Common.hpp"

std::string error_username_inuse(std::string username) {
    return (":" + std::string(IRC_NAME) + " 433 * " + username + " : is already connected\r\n");
}

std::string error_nickname_inuse(std::string username) {
    return (":" + std::string(IRC_NAME) + " 433 * " + username + " :nickname is already in use\r\n");
}

std::string error_nosuchnick(std::string nickname, std::string channel) {
    return (":" + std::string(IRC_NAME) + " 401 " + nickname + " " + channel + " :no such nick/channel\r\n");
}

std::string error_nosuchchannel(std::string username, std::string channel) {
    return (":" + std::string(IRC_NAME) + " 403 * " + username + " " + channel + " :no such channel\r\n");
}

std::string error_notonchannel(std::string nickname, std::string channel) {
    return (":" + std::string(IRC_NAME) + " 442 " + nickname + " " + channel + " :you're not on that channel\r\n");
}

std::string error_useronchannel(std::string nickname, std::string target, std::string channel) {
    return (":" + std::string(IRC_NAME) + " 443 " + nickname + " " + target + " " + channel
            + " :is already on channel\r\n");
}

std::string error_nosuchserver(std::string nickname, std::string target) {
    return (":" + std::string(IRC_NAME) + " 402 " + nickname + " " + target + " :no such server\r\n");
}

std::string error_needmoreparams(std::string nickname, std::string command) {
    return (":" + std::string(IRC_NAME) + " 461 " + nickname + " " + command + " :not enough parameters\r\n");
}

std::string error_chanoprivsneeded(std::string nickname, std::string channel) {
    return (":" + std::string(IRC_NAME) + " 482 " + nickname + " " + channel + " :you're not channel operator\r\n");
}

std::string error_notregistered(std::string nickname) {
    return (":" + std::string(IRC_NAME) + " 451 " + nickname + " :you have not registered\r\n");
}

std::string error_passwdmismatch(std::string nickname) {
    nickname = nickname.empty() ? "*" : nickname;
    return (":" + std::string(IRC_NAME) + " 464 " + nickname + " :password incorrect\r\n");
}

/*std::string error_cannotsendtochan(std::string username, std::string channel) {
    return (":" + std::string(IRC_NAME) + " 404 * " + username + " " + channel + " :Cannot send to channel\r\n");
}*/
