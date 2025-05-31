#include "Channel.hpp"

Channel::Channel(std::string name, int fd)
    : channel_name(name), topic("Regular channel, nothing special!"), perm_invite(false), perm_topic(false) {
    users.push_back(fd);
    operators.push_back(fd);
}

void Channel::add_user(int fd, int mode) {
    if (mode == CH_ADD_USER)
        users.push_back(fd);
    else if (mode == CH_ADD_OPERATOR)
        operators.push_back(fd);
    else
        invites.push_back(fd);
}

void Channel::delete_user(int fd) {
    for (size_t i = 0; i < users.size(); i++) {
        if (users[i] == fd)
            users.erase(users.begin() + i);
    }
}

std::string Channel::getName() { return channel_name; }
