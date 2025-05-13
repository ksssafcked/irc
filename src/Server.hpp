#pragma once

#include <string>

class Server {
private:
    Server() {}

public:
    Server(unsigned short port, const std::string& password);
    ~Server();

    void run();

private:
    unsigned short _port;
    std::string _password;
};
