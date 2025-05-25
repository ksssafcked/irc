#include "Server.hpp"

#include <algorithm>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <csignal>

#include "Common.hpp"

namespace utils {

struct FindClient {
    int fd;

    FindClient(int fd) : fd(fd) {}

    bool operator()(const Client& client) const { return client.fd == fd; }
};

} // namespace utils

bool Server::running = false;

Server::Server(unsigned short port, const std::string& password)
    : _initialized(false), _port(port), _password(password) {
    if (!this->setup_socket()) {
        return;
    }

    if (!this->setup_signal()) {
        return;
    }

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
            ClientIterator client = std::find_if(_clients.begin(), _clients.end(), utils::FindClient(_fds[i].fd));
            if (client == _clients.end()) {
                // NOTE: should never happen
                continue;
            }

            if (_fds[i].revents & POLLIN) {
                handle_client_input(client);
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

    Client client;
    client.fd = new_fd;
    client.poll_index = this->_fds.size() - 1;

    this->_clients.push_back(client);

    return true;
}

void Server::drop_client(ClientIterator& client) {
    std::cout << "[info][server]: dropping client (" << client->fd << ")\n";

    close(client->fd);

    this->_fds.erase(this->_fds.begin() + client->poll_index);
    this->_clients.erase(client);
}

void Server::handle_client_input(ClientIterator& client) {
    std::cout << "[info][server]: handling input from client (" << client->fd << ")\n";

    char buffer[512];
    ssize_t bytes_read = recv(client->fd, buffer, sizeof(buffer), MSG_DONTWAIT);

    if (bytes_read <= 0) {
        std::cout << "[error][server]: failed to read input from client (" << client->fd << ")\n";
        drop_client(client);
        return;
    }

    // build up the current message (https://modern.ircdocs.horse/#message-format)
    client->current_msg += std::string(buffer, bytes_read);

    // Look for the "\r\n" sequence to begin parsing the command
    size_t delimiter = client->current_msg.find("\r\n");
    if (delimiter == std::string::npos) {
        return;
    }

    // TODO: parse command
    std::cout << ANSI_CYAN << "[debug][server]: parsing client message: [" << client->current_msg << "]\n"
              << ANSI_RESET;

    client->current_msg.clear();
}
