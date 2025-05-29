#include "Channel.hpp"

Channel::Channel(std::string name, int fd): channel_name(name) {
    users.push_back(fd);
}

void Channel::add_user(int fd) {
    users.push_back(fd);
}

void Channel::delete_user(int fd)
{
    for (size_t i = 0; i < users.size(); i++) {
        if (users[i] == fd)
            users.erase(users.begin() + i);
    }
}

std::string Channel::getName() { return channel_name;}