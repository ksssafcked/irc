#pragma once

#include <iostream>

class Client {
private:
    Client();

    int _fd;
    std::string _username;

public:
    Client(int fd, std::string username);
    Client(int fd);
    ~Client();
};
