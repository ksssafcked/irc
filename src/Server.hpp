#pragma once

#include <poll.h>
#include <string>
#include <vector>
#include <map>

#include "Client.hpp"
#include "Commands.hpp"

class Server {
private:
    Server() {}

public:
    Server(unsigned short port, const std::string& password);
    ~Server();

    void run();

    static bool running;

private:
    bool _initialized;

    unsigned short _port;
    std::string _password;

    int _socket;

    std::vector< struct pollfd > _fds;
    std::map< int, Client > _client_map;

    typedef std::map< int, Client >::iterator ClientIterator;

    bool setup_socket();
    bool setup_signal();
    void add_fd_to_poll(int fd, int events);

    bool accept_client();
    void drop_client(ClientIterator& client);

    void send_content(int fd, std::string message);
    void send_to_all(ClientIterator& client, CommandResult result);

    bool handle_client_input(ClientIterator& client);
};
