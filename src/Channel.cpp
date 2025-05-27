#include "Channel.hpp"

Channel::Channel(std::string channel_name): _channel_name(channel_name) {}

void Channel::add_user(int fd) {
    _users.push_back(fd);
}

std::string Channel::getName() { return _channel_name;}