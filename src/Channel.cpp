#include "Channel.hpp"

Channel::Channel(std::string name, int fd)
    : channel_name(name), topic("Regular channel, nothing special!"), perm_invite(false), perm_topic(false),
      need_pass(false), has_limit(false), limit_user(0) {
    users.push_back(fd);
    operators.push_back(fd);
}

void Channel::add_user(int fd, int mode) {
    if (mode == CH_ADD_USER) {
        if (has_limit)
            if (users.size() == limit_user)
                return;
        users.push_back(fd);
    }

    else if (mode == CH_ADD_OPERATOR) {
        for (size_t i = 0; i < operators.size(); i++) {
            if (operators[i] == fd)
                return;
        }
        operators.push_back(fd);
    } else {
        for (size_t i = 0; i < invites.size(); i++) {
            if (invites[i] == fd)
                return;
        }
        invites.push_back(fd);
    }
}

void Channel::delete_user(int fd) {
    for (size_t i = 0; i < users.size(); i++) {
        if (users[i] == fd)
            users.erase(users.begin() + i);
    }
}

void Channel::delete_from_all(int fd) {
    for (size_t i = 0; i < users.size(); i++) {
        if (users[i] == fd) {
            users.erase(users.begin() + i);
            break;
        }
    }
    for (size_t j = 0; j < operators.size(); j++) {
        if (operators[j] == fd) {
            operators.erase(operators.begin() + j);
            break;
        }
    }
    for (size_t k = 0; k < invites.size();) {
        if (invites[k] == fd) {
            invites.erase(invites.begin() + k);
            continue;
        }
        k++;
    }
}

std::string Channel::getName() { return channel_name; }
