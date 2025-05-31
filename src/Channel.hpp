#pragma once

#include <vector>
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
    std::vector< int > users;
    std::vector< int > operators;
    std::vector< int > invites;

    bool perm_invite;
    bool perm_topic;

    Channel(std::string name, int fd);

    void add_user(int fd, int mode);
    void delete_user(int fd);
    std::string getName();

private:
    Channel();
};
