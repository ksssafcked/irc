#pragma once

#include <string>

struct Client {
    int fd;
    size_t poll_index;

    // data that the client sent to the server is stored here
    std::string current_msg;

    // if the client is fully connected or not
    bool registered;

    std::string username;
};
