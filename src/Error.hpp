#pragma once

std::string error_username_inuse(std::string username);
std::string error_nickname_inuse(std::string username);
std::string error_nosuchchannel(std::string username, std::string channel);
std::string error_cannotsendtochan(std::string username, std::string channel);
	