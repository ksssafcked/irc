#include "Server.hpp"
#include "Common.hpp"
#include "Commands.hpp"

#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <csignal>

bool Server::running = false;

Server::Server(unsigned short port, const std::string& password)
    : _initialized(false), _port(port), _password(password) {
    if (!this->setup_socket()) {
        return;
    }

    if (!this->setup_signal()) {
        return;
    }

    Commands::register_commands();

    std::cout << "[info][server]: listening on port " << port << "\n" << ANSI_RESET;

    _initialized = true;
}

Server::~Server() {
    std::cout << "[info][server]: shutdown\n";

    if (this->_socket) {
        close(this->_socket);
    }
}

void Server::run() {
    if (!_initialized) {
        std::cerr << ANSI_RED << "[error][server]: failed to run because it is not initialized!\n" << ANSI_RESET;
        return;
    }

    Server::running = true;

    // TODO: signal to interrupt this
    while (Server::running) {
        // check if anything happened
        if (poll(_fds.data(), _fds.size(), 0) < 0) {
            std::cerr << ANSI_RED << "[error][server]: failed to poll!\n" << ANSI_RESET;
            return;
        }

        // accept any incoming connections
        if (_fds[0].revents & POLLIN) {
            this->accept_client();
        }

        // handle any clients
        for (size_t i = 1; i < _fds.size(); i++) {
            ClientIterator client = _client_map.find(_fds[i].fd);
            if (client == _client_map.end()) {
                // NOTE: should never happen
                continue;
            }

            if (_fds[i].revents & POLLIN) {
                handle_client_input(client);
            }
            if (all_info(client->second) && !client->second.connected) {
                std::cout << ANSI_RED << "All info !\n" << ANSI_RESET;
                send_content(client->first, \
                    ":ircserver 001 " + client->second.nickname + " :Welcome to the IRC network, " + client->second.nickname + "!" + client->second.username + "@" + client->second.hostname + "\r\n");
                client->second.connected = true;
            }
            // TODO: POLLOUT
        }
    }
}

bool Server::setup_socket() {
    this->_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (this->_socket < 0) {
        std::cerr << ANSI_RED << "[error][server]: socket() failed!\n" << ANSI_RESET;
        return false;
    }

    int opt = 1;
    if (setsockopt(this->_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << ANSI_RED << "[error][server]: setsockopt() failed()!\n" << ANSI_RESET;
        return false;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(this->_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << ANSI_RED << "[error][server]: bin() failed!\n" << ANSI_RESET;
        return false;
    }

    if (listen(_socket, 10) == -1) {
        std::cerr << ANSI_RED << "[error][server]: listen() failed!\n" << ANSI_RESET;
        return false;
    }

    this->add_fd_to_poll(_socket, POLLIN);

    return true;
}

static void sigint_handler(int signum) {
    if (signum == SIGINT) {
        std::cout << "[info][server]: received interrupt signal\n";
        Server::running = false;
    }
}

bool Server::setup_signal() {
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        return false;
    }

    return true;
}

void Server::add_fd_to_poll(int fd, int events) {
    pollfd pfd;
    pfd.fd = fd;
    pfd.events = events;
    pfd.revents = 0;

    _fds.push_back(pfd);
}

bool Server::accept_client() {
    int new_fd = accept(_socket, 0, 0);
    if (new_fd < 0) {
        std::cerr << ANSI_RED << "[error][server]: failed to accept client!\n" << ANSI_RESET;
        return false;
    }

    std::cout << "[info][server]: accepting new client (" << new_fd << ")\n";

    this->add_fd_to_poll(new_fd, POLLIN | POLLOUT);

    _client_map[new_fd] = Client();

    return true;
}

void Server::drop_client(ClientIterator& client) {
    std::cout << "[info][server]: dropping client (" << client->first << ")\n";

    for (size_t i = 1; i < _fds.size(); ++i) {
        if (_fds[i].fd == client->first) {
            _fds.erase(this->_fds.begin() + i);
            break;
        }
    }

    close(client->first);
    _client_map.erase(client);
}

void Server::send_content(int fd, std::string message){
    send(fd, message.c_str(), message.length(), MSG_DONTWAIT);
}

void Server::send_to_all(ClientIterator& client, CommandResult result){
    if (result.type == CRD_RESULT_REPLY)
    {
        send_content(client->first, result.reply);
    }
    if (result.type == CRD_RESULT_BROADCAST)
    {
        std::string mess;
        mess = ":" + client->second.nickname + "!" + client->second.username + "@" + client->second.hostname + " PRIVMSG #ledion :" + result.message + "\r\n";
        for (std::vector< Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
        {
            for (std::vector< int>::iterator fds = (*it)._users.begin(); fds != (*it)._users.end(); fds++)
            {
                ClientIterator newclient = _client_map.find((*fds));
                if (client->first != newclient->first)
                {
                    std::cout << ANSI_RED << mess << ANSI_RESET;
                    send_content(newclient->first, mess);
                }
            }
        }
    }
}

bool Server::all_info(Client& client){ return (client.registered && !client.username.empty() && !client.nickname.empty()); }

bool Server::handle_client_input(ClientIterator& client) {
    // std::cout << "[info][server]: handling input from client (" << client->first << ")\n";

    char buffer[512];
    ssize_t bytes_read = recv(client->first, buffer, sizeof(buffer), MSG_DONTWAIT);

    if (bytes_read <= 0) {
        drop_client(client);
        return false;
    }

    // build up the current message (https://modern.ircdocs.horse/#message-format)
    client->second.input += std::string(buffer, bytes_read);
    // std::cout << ANSI_PINK << "[debug][server]: input from client (" << client->second.input << ")\n" << ANSI_RESET;


    CommandResult result;
    while (!client->second.input.empty())
    {
        std::cout << ANSI_PINK << "[debug][server]: input from client (" << client->second.input << ")\n" << ANSI_RESET;
        if (client->second.process_input(result))
        {
            if (result.type == CMD_CHANNEL)
            {
                if (_channels.empty())
                {
                    Channel c(result.reply);
                    c.add_user(client->first);
                    _channels.push_back(c);
                }
                else
                    _channels[0].add_user(client->first);
            }
            else if (result.type != CMD_CHECK_USER)
                this->send_to_all(client, result);
            else
            {
                if (client->second.password == this->_password)
                {
                    client->second.registered = true;
                    std::cout << ANSI_RED << "BRAVO !\n" << ANSI_RESET;
                }
            }
            if (all_info(client->second))
                send_content(client->first, \
                ":ircserver 001 " + client->second.nickname + " :Welcome to the IRC network, " + client->second.nickname + "!" + client->second.username + "@" + client->second.hostname + "\r\n");
        }
    }

    return true;
}
