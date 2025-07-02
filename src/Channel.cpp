#include "Channel.hpp"

Channel::Channel(std::string name, int fd)
    : channel_name(name), topic("Regular channel, nothing special!"), perm_invite(false), perm_topic(false),
      need_pass(false), has_limit(false), limit_user(0) {
    users.insert(fd);
    operators.insert(fd);
}

void Channel::add_user(int fd, int mode) {
    if (mode == CH_ADD_USER) {
        if (has_limit)
            if (users.size() == limit_user)
                return;
        users.insert(fd);
    }

    else if (mode == CH_ADD_OPERATOR) {
        operators.insert(fd);
    } else {
        invites.insert(fd);
    }
}

void Channel::delete_user(int fd) { users.erase(fd); }

void Channel::delete_from_all(int fd) {
    users.erase(fd);
    operators.erase(fd);
    invites.erase(fd);
}

std::string Channel::getName() { return channel_name; }
