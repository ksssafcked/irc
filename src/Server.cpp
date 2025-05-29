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

    int activity = 0;
    // TODO: signal to interrupt this
    while (Server::running) {
        // check if anything happened
        activity = poll(_fds.data(), _fds.size(), -1);
        if (activity < 0) {
            std::cerr << ANSI_RED << "[error][server]: failed to poll!\n" << ANSI_RESET;
            return;
        }
        else if (activity == 0)
            continue;
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
                if (!handle_client_input(client))
                    break;
            }
            if (all_info(client->second) && !client->second.connected) {
                std::cout << ANSI_RED << "All info !\n" << ANSI_RESET;
                send_message(client->first, \
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
        std::cerr << ANSI_RED << "[error][server]: bind() failed!\n" << ANSI_RESET;
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
    sigemptyset(&sa.sa_mask);
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
    delete_user_from_channels(client->first);
    _client_map.erase(client);
}

void Server::send_message(int fd, std::string message){
    send(fd, message.c_str(), message.length(), MSG_DONTWAIT);
}

void Server::send_to_all(ClientIterator& client, CommandResult result) {
    if (result.type == CMD_RESULT_REPLY)
    {
        send_message(client->first, result.reply);
    }
    if (result.type == CMD_RESULT_BROADCAST) {
        std::string mess;
        mess = ":" + client->second.nickname + "!" + client->second.username + "@" + client->second.hostname + " PRIVMSG " + result.reply + " :" + result.message + "\r\n";
        for (size_t i = 0; i < _channels.size();i++)
        {
            if (_channels[i].channel_name != result.reply)
                continue;
            std::vector < int > user_list = _channels[i].users;
            for (size_t j = 0; j < user_list.size(); j++)
            {
                if (client->first != user_list[j])
                {
                    std::cout << ANSI_RED << mess << ANSI_RESET;
                    send_message(user_list[j], mess);
                }
            }
        }
    }
}

// void Server::send_to_all(ClientIterator& client, CommandResult result){
//     if (result.type == CMD_RESULT_REPLY)
//     {
//         send_message(client->first, result.reply);
//     }
//     if (result.type == CMD_RESULT_BROADCAST)
//     {
//         std::string mess;
//         mess = ":" + client->second.nickname + "!" + client->second.username + "@" + client->second.hostname + " PRIVMSG #" + result.reply + " :" + result.message + "\r\n";
//         for (std::vector< Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
//         {
//             for (std::vector< int>::iterator fds = (*it).users.begin(); fds != (*it).users.end(); fds++)
//             {
//                 ClientIterator newclient = _client_map.find((*fds));
//                 if (client->first != newclient->first)
//                 {
//                     std::cout << ANSI_RED << mess << ANSI_RESET;
//                     send_message(newclient->first, mess);
//                 }
//             }
//         }
//     }
// }

bool Server::all_info(Client& client){ return (client.registered && !client.username.empty() && !client.nickname.empty()); }

bool Server::does_channel_exist(std::string name) {
    for (size_t i = 0; i < _channels.size(); i++) {
        if (_channels[i].channel_name == name)
            return true;
    }
    return false;
}

void Server::add_user_to_channel(std::string name, int fd) {
    for (size_t i = 0; i < _channels.size(); i++) {
        if (_channels[i].channel_name == name) {
            _channels[i].add_user(fd);
        }
    }
}

void Server::delete_user_from_channels(int fd) {
    for (size_t i = 0; i < _channels.size(); i++) {
        for (size_t j = 0; j < _channels[i].users.size(); j++) {
            if (_channels[i].users[j] == fd) {
                _channels[i].users.erase(_channels[i].users.begin() + j);
            }
        }
    }
}

bool Server::handle_client_input(ClientIterator& client) {
    // std::cout << "[info][server]: handling input from client (" << client->first << ")\n";

    char buffer[512];
    ssize_t bytes_read = recv(client->first, buffer, sizeof(buffer), MSG_DONTWAIT);

    if (bytes_read <= 0) {
        drop_client(client);
        return false;
    }

    // build up the current message (https://modern.ircdocs.horse/#message-format)
    client->second.input = std::string(buffer, bytes_read);
    // std::cout << ANSI_PINK << "[debug][server]: input from client (" << client->second.input << ")\n" << ANSI_RESET;

    std::cout << ANSI_RED << "channels :" << _channels.size() << std::endl;
    if (_channels.size() > 0)
        std::cout << ANSI_RED << "users :" << _channels[0].users.size() << std::endl << ANSI_RESET;


    CommandResult result;
    while (!client->second.input.empty())
    {
        std::cout << ANSI_PINK << "[debug][server]: input from client (" << client->second.input << ")\n" << ANSI_RESET;
        if (client->second.process_input(result))
        {
            if (result.type == CMD_CHANNEL)
            {
                if (does_channel_exist(result.reply))
                    add_user_to_channel(result.reply, client->first);
                else {
                    Channel c(result.reply, client->first);
                    _channels.push_back(c);
                }
            } else if (result.type == CMD_QUIT){
                drop_client(client);
                return false;
            }
            else if (result.type != CMD_CHECK_USER)
                send_to_all(client, result);
            else
            {
                if (client->second.password == this->_password)
                {
                    client->second.registered = true;
                    std::cout << ANSI_RED << "BRAVO !\n" << ANSI_RESET;
                }
            }
        }
    }

    return true;
}
