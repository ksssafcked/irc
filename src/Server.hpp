#pragma once

#include <poll.h>
#include <string>
#include <vector>
#include <map>

#include "Client.hpp"
#include "Commands.hpp"
#include "Channel.hpp"

#define IRC_NAME "burekserv"

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
    std::vector< Channel > _channels;

    typedef std::map< int, Client >::iterator ClientIterator;

    bool setup_socket();
    bool setup_signal();
    void add_fd_to_poll(int fd, int events);

    bool accept_client();
    void drop_client(ClientIterator& client);

    void send_message(int fd, std::string message);
    void send_to_all(ClientIterator& client, CommandResult result);
    bool does_user_exist(std::string name, int& fd);
    bool does_channel_exist(std::string name);
    bool can_user_invite(ClientIterator& client, std::string channel);
    void handle_channel(ClientIterator& client, CommandResult result);

    void check_infos(ClientIterator& client, CommandResult result);
    void delete_channel(size_t i);
    void change_mode(ClientIterator& client, CommandResult result);
    bool all_info(Client& client);
    void add_user_to_channel(std::string name, int fd, int mode);
    void delete_user_from_channels(int fd);
    bool handle_client_input(ClientIterator& client);
};
