#pragma once

#include <vector>
#include <string>
#include <iostream>

enum ChannelMode {
    CH_NO_INVITE,
    CH_INVITE,
    CH_ADD_USER,
    CH_ADD_OPERATOR,
    CH_ADD_INVITE,
};

struct Channel {

    std::string channel_name;
    std::vector< int > users;
    std::vector< int > operators;
    std::vector< int > invites;

    int mode;

    Channel(std::string name, int fd);

    void add_user(int fd, int mode);
    void delete_user(int fd);
    std::string getName();

private:
    Channel();
};
