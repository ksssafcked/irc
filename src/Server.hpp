#pragma once

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <string>
#include <sys/socket.h>
#include <poll.h>
#include <vector>

class Server {
private:
    Server() {}

    std::vector< struct pollfd >::iterator deleteClient(std::vector< struct pollfd >::iterator it);

    std::string _port;
    std::string _password;
    int server_fd;
    int _user_nb;
    std::vector< struct pollfd > _clients;

public:
    Server(std::string port, const std::string& password);
    ~Server();
    void add_to_pfds(struct pollfd* pfds[], int newfd, int* fd_count, int* fd_size);
    int open_socket();
    void run();
};
