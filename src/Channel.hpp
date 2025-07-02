#pragma once

#include <set>
#include <string>
#include <iostream>

enum ChannelMode {
    CH_ADD_USER,
    CH_ADD_OPERATOR,
    CH_ADD_INVITE,
};

struct Channel {

    std::string channel_name;
    std::string topic;
    std::string password;

    std::set< int > users;
    std::set< int > operators;
    std::set< int > invites;

    bool perm_invite;
    bool perm_topic;
    bool need_pass;
    bool has_limit;

    size_t limit_user;

    Channel(std::string name, int fd);

    void add_user(int fd, int mode);
    void delete_user(int fd);
    void delete_from_all(int fd);
    std::string getName();

private:
    Channel();
};
