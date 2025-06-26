#include "Server.hpp"
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
        } else if (activity == 0)
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
            // if (client->second.state == S_CONNECT) {
            //

            //     client->second.connected = true;
            // }
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

void Server::send_message(int fd, std::string message) {
    send(fd, message.c_str(), message.length(), MSG_DONTWAIT);
    std::cout << ANSI_RED << "[debug][server]: server sent to " << fd << ": [" << message << "]\n" << ANSI_RESET;
}

void Server::burek_to_all(std::string& channel) {
    std::string mess;
    mess = ":burek!burek@burek PRIVMSG " + channel + " :burek\r\n";
    for (size_t i = 0; i < _channels.size(); i++) {
            if (_channels[i].channel_name != channel)
                continue;
            std::vector< int > user_list = _channels[i].users;
            for (size_t j = 0; j < user_list.size(); j++) {
                send_message(user_list[j], mess);
            }
        }
}

void Server::send_to_all(ClientIterator& client, CommandResult result) {
    if (result.type == CMD_RESULT_REPLY) {
        send_message(client->first, result.reply);
    }
    if (result.type == CMD_RESULT_BROADCAST) {
        std::string mess;
        mess = ":" + client->second.nickname + "!" + client->second.username + "@" + client->second.hostname
               + " PRIVMSG " + result.reply + " :" + result.message + "\r\n";
        for (size_t i = 0; i < _channels.size(); i++) {
            if (_channels[i].channel_name != result.reply)
                continue;
            std::vector< int > user_list = _channels[i].users;
            for (size_t j = 0; j < user_list.size(); j++) {
                if (client->first != user_list[j]) {
                    std::cout << ANSI_RED << mess << ANSI_RESET;
                    send_message(user_list[j], mess);
                }
            }
        }
        std::size_t burek = result.message.find("burek");
        if (burek != std::string::npos) {
            burek_to_all(result.reply);
        }
    }
}

bool Server::all_info(Client& client) {
    return (!client.username.empty() && !client.hostname.empty() && !client.nickname.empty());
}

bool Server::does_channel_exist(std::string name) {
    for (size_t i = 0; i < _channels.size(); i++) {
        if (_channels[i].channel_name == name)
            return true;
    }
    return false;
}

bool Server::does_user_exist(std::string name, int& fd) {
    for (size_t i = 1; i < _fds.size(); i++) {
        ClientIterator client = _client_map.find(_fds[i].fd);
        if (client == _client_map.end()) {
            continue;
        }
        if (client->second.nickname == name) {
            fd = client->first;
            return true;
        }
    }
    return false;
}

void Server::add_user_to_channel(CommandResult result, int fd, int mode) {
    std::string name = result.reply;
    for (size_t i = 0; i < _channels.size(); i++) {
        if (mode == CH_ADD_USER) { // add user in users vector
            if (_channels[i].channel_name != name)
                continue;
            if (!_channels[i].perm_invite) {
                for (size_t j = 0; j < _channels[i].invites.size(); j++) {
                    if (_channels[i].invites[j] == fd) {
                        _channels[i].invites.erase(_channels[i].invites.begin() + j);
                        _channels[i].add_user(fd, mode);
                        return;
                    }
                }
                if (_channels[i].need_pass) {
                    if (result.extra == _channels[i].password) {
                        _channels[i].add_user(fd, mode);
                    }
                } else {
                    _channels[i].add_user(fd, mode);
                }
            } else {
                for (size_t j = 0; j < _channels[i].invites.size(); j++) {
                    if (_channels[i].invites[j] == fd) {
                        _channels[i].add_user(fd, mode);
                        _channels[i].invites.erase(_channels[i].invites.begin() + j);
                        return;
                    }
                }
            }
        } else { // add user in operators/invites vector
            if (_channels[i].channel_name == name)
                _channels[i].add_user(fd, mode);
        }
    }
}

void Server::kick_user(size_t i, int fd, Client& client, std::string message) {
    std::string reply = ":" + client.nickname + "!" + client.username + "@" + client.hostname + " KICK "
                        + _channels[i].channel_name + " " + _client_map[fd].nickname + " " + message + "\r\n";
    send_message(fd, reply);
    _channels[i].delete_from_all(fd);
    if (_channels[i].users.size() == 0 || _channels[i].operators.size() == 0)
        delete_channel(i);
}

void Server::kick_user(size_t i, int fd, std::string user, std::string message) {
    std::string reply =
        ":" + user + " KICK " + _channels[i].channel_name + " " + _client_map[fd].nickname + " " + message + "\r\n";
    send_message(fd, reply);
    _channels[i].delete_from_all(fd);
    if (_channels[i].users.size() == 0 || _channels[i].operators.size() == 0)
        delete_channel(i);
}

bool Server::is_user_operator(ClientIterator& client, Channel& channel) {
    for (size_t i = 0; i < channel.operators.size(); i++) {
        if (channel.operators[i] == client->first) {
            return true;
        }
    }
    return false;
}

void Server::check_kick(ClientIterator& client, CommandResult result) {
    int fd;
    if (!does_channel_exist(result.reply) || !does_user_exist(result.message, fd))
        return;
    if (fd == client->first)
        return;
    size_t i;
    for (i = 0; i < _channels.size(); i++) {
        if (_channels[i].channel_name == result.reply) {
            if (!is_user_operator(client, _channels[i]))
                return;
            break;
        }
    }
    for (size_t k = 0; k < _channels[i].users.size(); k++) {
        if (_channels[i].users[k] == fd) {
            kick_user(i, fd, client->second, result.extra);
            delete_user_from_channels(fd);
        }
    }
}

int Server::getfd_byname(std::string name) {
    for (ClientIterator it = _client_map.begin(); it != _client_map.end(); it++) {
        if (it->second.nickname == name)
            return (it->first);
    }
    return -1;
}

void Server::delete_channel(size_t i) {
    _channels[i].users.clear();
    _channels[i].operators.clear();
    _channels[i].invites.clear();
    _channels.erase(_channels.begin() + i);
}

void Server::delete_user_from_channels(int fd) {
    for (size_t i = 0; i < _channels.size();) {
        
        _channels[i].delete_from_all(fd);
        if (_channels[i].users.size() == 0 || _channels[i].operators.size() == 0) {
            std::cout << ANSI_RED << "Channel delete number: " << (i + 1) << "\n" << ANSI_RESET;
            for (size_t j = 0; j < _channels[i].users.size(); j++) {
                kick_user(i, _channels[i].users[j], std::string(IRC_NAME), ":All operators left the chat!");
    }
        }
        else
            i++;
        std::cout << ANSI_RED << "Channel size: " << _channels.size() << "\n" << ANSI_RESET;
    }
}

std::string Server::error_message(std::string nickname, int error_code, std::string error_message) {
    (void)error_code;
    std::string reply = ":" + std::string(IRC_NAME) + " 469 * " + nickname + " " + error_message + "\r\n";
    return reply;
}

void Server::check_infos(ClientIterator& client, CommandResult result) {
    std::cout << "Username : " << result.reply << std::endl;
    if (result.type == CMD_CHECK_PASS) { // if we check the password
        if (result.reply == this->_password) {
            client->second.state = S_USER;
            std::cout << ANSI_RED << "Good password!\n" << ANSI_RESET;
        }
    } else {                        // if we check for username or nickname
        if (result.message.empty()) // check only nickname
        {
            for (ClientIterator clients = _client_map.begin(); clients != _client_map.end(); clients++) {
                if (clients->first != client->first && clients->second.nickname == result.reply) {
                    send_message(client->first, error_nickname_inuse(result.reply));
                    return; // if the nickname is already used
                }
            }
            client->second.nickname = result.reply;
            std::cout << ANSI_RED << "New nick !\n" << ANSI_RESET;
        } else { // check username and hostname
            for (ClientIterator clients = _client_map.begin(); clients != _client_map.end(); clients++) {
                if (clients->first != client->first
                    && (clients->second.username == result.reply || clients->second.hostname == result.message)) {
                    send_message(client->first, error_username_inuse(result.reply));
                    return; // if the username is already used
                }
            }
            client->second.username = result.reply;
            client->second.hostname = result.message;
            std::cout << ANSI_RED << "New username !\n" << ANSI_RESET;
        }
        if (all_info(client->second)) {
            client->second.state++;
            std::string message = ":" + std::string(IRC_NAME) + " 001 " + client->second.nickname
                                  + " :Welcome to the IRC network, " + client->second.nickname + "!"
                                  + client->second.username + "@" + client->second.hostname + "\r\n";
            send_message(client->first, message);
            std::cout << ANSI_RED << "All info !\n" << ANSI_RESET;
        }
    }
}

bool Server::can_user_invite(ClientIterator& client, std::string channel) {
    size_t i;
    size_t j;
    for (i = 0; i < _channels.size(); i++) {
        if (_channels[i].channel_name == channel)
            break;
    }
    if (i == _channels.size())
        return false;
    if (_channels[i].perm_invite) {
        for (j = 0; j < _channels[i].operators.size(); j++) {
            if (_channels[i].operators[j] == client->first)
                return true;
        }
    } else {
        for (j = 0; j < _channels[i].users.size(); j++) {
            if (_channels[i].users[j] == client->first) {
                return true;
            }
        }
    }
    return false;
}

void Server::add_operator(ClientIterator& client, Channel& channel, CommandResult result) {
    int fd = getfd_byname(result.extra);
    std::cout << ANSI_RED << result.extra << ":" << fd << ANSI_RESET << std::endl;

    if (fd == -1 || fd == client->first)
        return;
    if (result.message == "+o") {
        for (size_t i = 0; i < channel.operators.size(); i++) {
            if (channel.operators[i] == fd)
                return;
            channel.operators.push_back(fd);
        }
    } else {
        for (size_t i = 0; i < channel.operators.size(); i++) {
            if (channel.operators[i] == fd)
                channel.operators.erase(channel.operators.begin() + i);
        }
    }
}

void Server::change_mode(ClientIterator& client, CommandResult result) {
    if (does_channel_exist(result.reply)) {
        size_t i;
        for (i = 0; i < _channels.size(); i++) {
            if (_channels[i].channel_name == result.reply)
                break;
        }
        std::cout << ANSI_RED << result.message << ANSI_RESET << std::endl;
        if (is_user_operator(client, _channels[i])) {
            if (result.message == "+i") {
                _channels[i].perm_invite = true;
                return;
            } else if (result.message == "-i") {
                _channels[i].perm_invite = false;
            } else if (result.message == "+k" && !result.extra.empty()) {
                _channels[i].need_pass = true;
                _channels[i].password = result.extra;
            } else if (result.message == "-k") {
                _channels[i].need_pass = false;
                _channels[i].password.clear();
            } else if (result.message == "+l") {
                _channels[i].limit_user = static_cast< size_t >(std::strtol(result.extra.c_str(), NULL, 10));
                std::cout << ANSI_RED << _channels[i].limit_user << ANSI_RESET << std::endl;
                _channels[i].has_limit = true;
            } else if (result.message == "-l") {
                _channels[i].has_limit = false;
                _channels[i].limit_user = 0;
            } else if (result.message == "+t") {
                _channels[i].perm_topic = true;
            } else if (result.message == "-t") {
                _channels[i].perm_topic = false;
            } else if (result.message == "+o" || result.message == "-o") {
                add_operator(client, _channels[i], result);
            }
        }
    }
}

void Server::handle_channel(ClientIterator& client, CommandResult result) {
    if (result.type == CMD_CHANNEL) {
        if (does_channel_exist(result.reply))
            add_user_to_channel(result, client->first, CH_ADD_USER);
        else {
            Channel c(result.reply, client->first);
            _channels.push_back(c);
        }
        std::cout << ANSI_RED << "channels :" << _channels.size() << std::endl;
        if (_channels.size() > 0)
            std::cout << ANSI_RED << "users :" << _channels[0].users.size() << std::endl << ANSI_RESET;
    } else {
        int fd;
        std::string mess;
        if (does_channel_exist(result.reply)) {
            if (can_user_invite(client, result.reply) && does_user_exist(result.message, fd)) {
                mess = ":" + client->second.username + "!" + _client_map[fd].username + "@" + _client_map[fd].hostname;
                //+ "!" + _client_map[fd].username + "@" + _client_map[fd].hostname
                mess += " INVITE " + result.message + " " + result.reply + "\r\n";
                // mess = ":server JOIN " + result.message + "\r\n";
                send_message(fd, mess);
                add_user_to_channel(result, fd, CH_ADD_INVITE);
            }
        } else { // TODO : error channel doesn't exist}
        }
    }
}

void Server::topic_handle(ClientIterator& client, CommandResult result) {
    if (result.message.empty()) {

    } else {
        if (!does_channel_exist(result.reply)) {
            // send_message(client->first, error_nosuchchannel(client->second.nickname, result.reply));
            return;
        } else {
            for (size_t i = 0; i < _channels.size(); i++) {
                if (_channels[i].channel_name == result.reply) {
                    if (!_channels[i].perm_topic) {
                        for (size_t j = 0; j < _channels[i].users.size(); j++) {
                            if (_channels[i].users[j] == client->first) {
                                _channels[i].topic = result.message;
                                std::string mess =
                                    ":" + std::string(IRC_NAME) + " 332 " + client->second.nickname + " ";
                                mess += result.reply + " :" + _channels[i].topic + "\r\n";
                                std::cout << ANSI_RED << "topic: " << _channels[i].topic << "\n" << ANSI_RESET;
                                send_message(client->first, mess);
                            }
                        }
                    } else {
                        for (size_t j = 0; j < _channels[i].operators.size(); j++) {
                            if (_channels[i].operators[j] == client->first) {
                                _channels[i].topic = result.message;
                                std::string mess =
                                    ":" + std::string(IRC_NAME) + " 332 " + client->second.nickname + " ";
                                mess += result.reply + " :" + _channels[i].topic + "\r\n";
                                std::cout << ANSI_RED << "topic: " << _channels[i].topic << "\n" << ANSI_RESET;
                                send_message(client->first, mess);
                            }
                        }
                    }
                }
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

    for (std::string::iterator it = client->second.input.begin(); it != client->second.input.end(); it++) {
        if (*it >= '\x01' && *it <= '\x07') {
            client->second.input.erase(it);
            it--;
        }
    }

    CommandResult result;
    while (!client->second.input.empty()) {
        std::cout << ANSI_PINK << "[debug][server]: input from client (" << client->second.input << ")\n" << ANSI_RESET;
        if (client->second.process_input(result)) {
            if (result.type == CMD_RESULT_REPLY)
                send_message(client->first, result.reply);
            else if (result.type == CMD_CHANNEL || result.type == CMD_INVITE)
                handle_channel(client, result);
            else if (result.type == CMD_QUIT) {
                drop_client(client);
                return false;
            } else if (result.type == CMD_CHECK_USER || result.type == CMD_CHECK_PASS) {
                check_infos(client, result);
            } else if (result.type == CMD_CHANGE_MODE) {
                change_mode(client, result);
            } else if (result.type == CMD_KICK) {
                check_kick(client, result);
            } else if (result.type == CMD_TOPIC) {
                topic_handle(client, result);
            } else
                send_to_all(client, result);
        }
    }

    return true;
}
