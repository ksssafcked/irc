#pragma once
#include <string>

std::string error_username_inuse(std::string username);
std::string error_nickname_inuse(std::string nickname);
std::string error_nosuchnick(std::string nickname, std::string channel);
std::string error_nosuchchannel(std::string nickname, std::string channel);
// std::string error_cannotsendtochan(std::string nickname, std::string channel);
std::string error_notonchannel(std::string nickname, std::string channel);
std::string error_useronchannel(std::string nickname, std::string target, std::string channel);
std::string error_needmoreparams(std::string nickname, std::string channel);
std::string error_chanoprivsneeded(std::string nickname, std::string channel);
std::string error_notregistered(std::string nickname);
std::string error_nosuchserver(std::string nickname, std::string target);
std::string error_passwdmismatch(std::string nickname);
std::string error_erroneusnickname(std::string nickname, std::string to_change);
