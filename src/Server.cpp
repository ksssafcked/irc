#include "Server.hpp"
#include "Commands.hpp"

#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <csignal>
#include <string>

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
    for (size_t i = 1; i < _fds.size(); i++) {
        close(_fds[i].fd);
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
    std::cout << "[info][server]: server sent to " << fd << ": [" << message << "]\n";
}

void Server::burek_to_all(std::string& channel) {
    std::string mess;
    mess = ":burek!burek@burek PRIVMSG " + channel + " :burek\r\n";

    size_t i;
    if (!does_channel_exist(channel, i))
        return;
    for (std::set< int >::const_iterator it = _channels[i].users.begin(); it != _channels[i].users.end(); it++) {
        send_message(*it, mess);
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
        size_t i;

        if (!does_channel_exist(result.reply, i))
            return;
        if (!_channels[i].users.count(client->first)) {
            send_message(client->first, error_notonchannel(client->second.nickname, result.reply));
            return;
        }
        const std::set< int >& user_list = _channels[i].users;
        std::set< int >::const_iterator it;
        for (it = user_list.begin(); it != user_list.end(); it++) {
            if (client->first != *it) {
                std::cout << ANSI_RED << mess << ANSI_RESET;
                send_message(*it, mess);
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

bool Server::does_channel_exist(std::string name, size_t& index) {
    for (size_t i = 0; i < _channels.size(); i++) {
        if (_channels[i].channel_name == name) {
            index = i;
            return true;
        }
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

void Server::send_join_message(Channel& channel, int fd) {
    ClientIterator client = _client_map.find(fd);
    if (client == _client_map.end())
        return;
    std::string message = ":" + client->second.nickname + "!" + client->second.username + "@" + client->second.hostname
                          + " JOIN " + channel.channel_name + "\r\n";
    for (std::set< int >::const_iterator it = channel.users.begin(); it != channel.users.end(); it++) {
        send_message(*it, message);
    }

    std::string topic = ":" + std::string(IRC_NAME) + " 332 " + _client_map[fd].nickname + " " + channel.channel_name
                        + " :" + channel.topic + "\r\n";
    send_message(fd, topic);

    std::string names;

    for (std::set< int >::const_iterator it = channel.users.begin(); it != channel.users.end(); it++) {
        int fd = *it;
        if (it != channel.users.begin())
            names += " ";
        ClientIterator user = _client_map.find(fd);
        if (user != _client_map.end()) {
            if (channel.operators.count(fd))
                names += "@";
            names += user->second.nickname;
        }
    }

    std::string names_rpl = ":" + std::string(IRC_NAME) + " 353 " + client->second.nickname + " = "
                            + channel.channel_name + " :" + names + "\r\n";
    send_message(fd, names_rpl);

    std::string end_names = ":" + std::string(IRC_NAME) + " 366 " + client->second.nickname + " " + channel.channel_name
                            + " :End of /NAMES list.\r\n";
    send_message(fd, end_names);
}

bool Server::add_user_to_channel(CommandResult result, int fd, int mode) {
    std::string name = result.reply;
    size_t i;
    if (!does_channel_exist(result.reply, i))
        return false;
    if (mode == CH_ADD_USER) { // add user in users vector
        if (_channels[i].users.count(fd)) {
            return false;
        }
        if (_channels[i].has_limit && _channels[i].users.size() == _channels[i].limit_user)
            return false;
        if (!_channels[i].perm_invite) {
            if (_channels[i].invites.count(fd)) {
                _channels[i].invites.erase(fd);
                _channels[i].users.insert(fd);
                return true;
            }
            if (_channels[i].need_pass) {
                if (result.extra == _channels[i].password) {
                    _channels[i].users.insert(fd);
                    return true;
                }
            } else {
                _channels[i].users.insert(fd);
                return true;
            }
        } else {
            if (_channels[i].invites.count(fd)) {
                _channels[i].invites.erase(fd);
                _channels[i].users.insert(fd);
                return true;
            }
        }
    } else { // add user in operators/invites vector
        _channels[i].add_user(fd, mode);
        return false;
    }
    return false;
}

void Server::kick_user(size_t i, int fd, Client& client, std::string message) {
    std::string reply = ":" + client.nickname + "!" + client.username + "@" + client.hostname + " KICK "
                        + _channels[i].channel_name + " " + _client_map[fd].nickname + " " + message + "\r\n";
    send_message(fd, reply);
    _channels[i].delete_from_all(fd);
    if (_channels[i].users.size() == 0 || _channels[i].operators.size() == 0) {
        kick_all_users_from_channel(_channels[i]);
        delete_channel(i);
    }
}

void Server::kick_user(size_t i, int fd, std::string user, std::string message) {
    std::string reply =
        ":" + user + " KICK " + _channels[i].channel_name + " " + _client_map[fd].nickname + " " + message + "\r\n";
    send_message(fd, reply);
    _channels[i].delete_from_all(fd);
    if (_channels[i].users.size() == 0 || _channels[i].operators.size() == 0) {
        kick_all_users_from_channel(_channels[i]);
        delete_channel(i);
    }
}

void Server::kick_all_users_from_channel(Channel& channel) {
    std::string message = ":All operators left the chat!";

    std::set< int > users_to_kick = channel.users;
    std::set< int >::const_iterator it;

    for (it = users_to_kick.begin(); it != users_to_kick.end(); ++it) {
        int user_fd = *it;

        if (_client_map.count(user_fd)) {
            std::string reply = ":" + std::string(IRC_NAME) + " KICK " + channel.channel_name + " "
                                + _client_map[user_fd].nickname + " " + message + "\r\n";
            send_message(user_fd, reply);
        }

        channel.delete_from_all(user_fd);
    }
}

bool Server::is_user_operator(ClientIterator& client, Channel& channel) {
    return channel.operators.count(client->first);
}

void Server::check_kick(ClientIterator& client, CommandResult result) {
    int fd;
    size_t i;
    if (!does_channel_exist(result.reply, i))
        return;
    else if (!does_user_exist(result.message, fd)) {
        send_message(client->first, error_nosuchnick(client->second.nickname, result.message));
    } else if (fd == client->first) {
        return;
    } else if (!_channels[i].users.count(client->first)) {
        send_message(client->first, error_notonchannel(client->second.nickname, _channels[i].channel_name));
    } else if (!is_user_operator(client, _channels[i])) {
        send_message(client->first, error_chanoprivsneeded(client->second.nickname, _channels[i].channel_name));
    } else if (_channels[i].users.count(fd))
        kick_user(i, fd, client->second, result.extra);
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
            kick_all_users_from_channel(_channels[i]);
            delete_channel(i);
        } else
            i++;
    }
}

std::string Server::error_message(std::string nickname, int error_code, std::string error_message) {
    (void)error_code;
    std::string reply = ":" + std::string(IRC_NAME) + " 469 * " + nickname + " " + error_message + "\r\n";
    return reply;
}

void Server::check_infos(ClientIterator& client, CommandResult result) {
    if (result.type == CMD_CHECK_PASS) { // if we check the password
        if (result.reply == this->_password) {
            client->second.state = S_USER; // TODO: send error to fd
        } else {
            send_message(client->first, error_passwdmismatch(client->second.nickname));
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
        }
        if (client->second.state != S_CONNECT && all_info(client->second)) {
            client->second.state++;
            std::string message = ":" + std::string(IRC_NAME) + " 001 " + client->second.nickname
                                  + " :Welcome to the IRC network, " + client->second.nickname + "!"
                                  + client->second.username + "@" + client->second.hostname + "\r\n";
            send_message(client->first, message);
        }
    }
}

bool Server::can_user_invite(ClientIterator& client, std::string channel) {
    size_t i;

    for (i = 0; i < _channels.size(); i++) {
        if (_channels[i].channel_name == channel)
            break;
    }
    if (i == _channels.size())
        return false;
    if (_channels[i].perm_invite) {
        if (_channels[i].operators.count(client->first))
            return true;
    } else {
        if (_channels[i].users.count(client->first))
            return true;
    }
    return false;
}

void Server::add_operator(ClientIterator& client, Channel& channel, CommandResult result) {
    int fd = getfd_byname(result.extra);

    if (fd == -1 || fd == client->first)
        return;
    else if (!channel.users.count(fd)) {
        send_message(client->first, error_notonchannel(client->second.nickname, result.extra));
    } else if (result.message == "+o") {
        channel.operators.insert(fd);
    } else {
        channel.operators.erase(fd);
    }
}

void Server::change_mode(ClientIterator& client, CommandResult result) {
    size_t i;
    if (!utils::is_channel(result.reply)) {
        return;
    }
    if (does_channel_exist(result.reply, i)) {
        if (is_user_operator(client, _channels[i])) {
            if (result.message == "+i") {
                _channels[i].perm_invite = true;
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
        } else {
            send_message(client->first, error_chanoprivsneeded(client->second.nickname, _channels[i].channel_name));
        }
    } else {
        send_message(client->first, error_nosuchchannel(client->second.nickname, result.reply));
    }
}

void Server::handle_channel(ClientIterator& client, CommandResult result) {
    if (result.type == CMD_CHANNEL) {
        size_t index;
        if (does_channel_exist(result.reply, index)) {
            if (add_user_to_channel(result, client->first, CH_ADD_USER) == true) {
                send_join_message(_channels[index], client->first);
            }
        } else {
            Channel c(result.reply, client->first);
            send_join_message(c, client->first);
            _channels.push_back(c);
        }
    } else {
        int fd;
        size_t index;
        (void)index;
        std::string mess;
        if (does_channel_exist(result.reply, index)) {
            if (can_user_invite(client, result.reply)) { // TODO: check if user is not in channel or if user is not ope
                                                         // and change error message depending the result
                if (does_user_exist(result.message, fd)) {
                    mess =
                        ":" + client->second.username + "!" + _client_map[fd].username + "@" + _client_map[fd].hostname;
                    mess += " INVITE " + result.message + " " + result.reply + "\r\n";
                    send_message(fd, mess);
                    add_user_to_channel(result, fd, CH_ADD_INVITE);
                } else { // target user doesnt exist
                    send_message(client->first, error_nosuchnick(client->second.nickname, result.message));
                }
            } else { // user is not operator
                send_message(client->first,
                             error_chanoprivsneeded(client->second.nickname, _channels[index].channel_name));
            }
        } else { // TODO : error channel doesn't exist}
            send_message(client->first, error_nosuchchannel(client->second.nickname, result.reply));
        }
    }
}

void Server::handle_nick(ClientIterator& client, CommandResult result) {
    for (ClientIterator clients = _client_map.begin(); clients != _client_map.end(); clients++) {
        if (clients->first != client->first && clients->second.nickname == result.reply) {
            send_message(client->first, error_nickname_inuse(result.reply));
            return; // if the nickname is already used
        }
    }
    std::string old_nick = client->second.nickname;
    client->second.nickname = result.reply;
    std::string nick_notif = ":" + old_nick + "!" + client->second.username + "@" + client->second.hostname
                             + " NICK :" + client->second.nickname + "\r\n";
    send_message(client->first, nick_notif);
    for (size_t i = 0; i < _channels.size(); i++) {
        if (_channels[i].users.count(client->first)) {
            const std::set< int >& user_list = _channels[i].users;
            std::set< int >::const_iterator it;
            for (it = user_list.begin(); it != user_list.end(); it++) {
                if (client->first != *it) {
                    send_message(*it, nick_notif);
                }
            }
        }
    }
}

void Server::topic_handle(ClientIterator& client, CommandResult result) {
    size_t i;
    if (!does_channel_exist(result.reply, i))
        return;
    if (result.message.empty()) {
        std::string topic_reply = ":" + std::string(IRC_NAME) + " 332 " + client->second.nickname + " "
                                  + _channels[i].channel_name + " :" + _channels[i].topic + "\r\n";
        send_message(client->first, topic_reply);
    } else {
        if (!_channels[i].users.count(client->first)) {
            send_message(client->first, error_notonchannel(client->second.nickname, _channels[i].channel_name));
        } else {
            if (_channels[i].perm_topic && !_channels[i].operators.count(client->first)) {
                send_message(client->first, error_chanoprivsneeded(client->second.nickname, _channels[i].channel_name));
            } else {
                _channels[i].topic = result.message;
                std::string mess = ":" + std::string(IRC_NAME) + " 332 " + client->second.nickname + " ";
                mess += result.reply + " :" + _channels[i].topic + "\r\n";
                for (std::set< int >::iterator it = _channels[i].users.begin(); it != _channels[i].users.end(); it++)
                    send_message(*it, mess);
            }
        }
    }
}

void Server::leave_channel(ClientIterator& client, CommandResult result) {
    size_t i;

    if (!does_channel_exist(result.reply, i)) {
        return;
    }
    if (_channels[i].users.count(client->first)) {
        std::string mess = ":" + client->second.nickname + "!" + client->second.username + "@" + client->second.hostname
                           + " PART " + result.reply + " :leaving\r\n";
        for (std::set< int >::const_iterator it = _channels[i].users.begin(); it != _channels[i].users.end(); it++)
            send_message(*it, mess);
        _channels[i].delete_from_all(client->first);
        if (!_channels[i].users.size() || !_channels[i].operators.size()) {
            kick_all_users_from_channel(_channels[i]);
            delete_channel(i);
        }
        return;
    }
}

void Server::ping_user(ClientIterator& client, CommandResult result) {
    std::string message;

    if (result.reply != std::string(IRC_NAME))
        message = error_nosuchserver(client->second.nickname, result.reply);
    else
        message = ":" + std::string(IRC_NAME) + " PONG " + std::string(IRC_NAME) + " :" + std::string(IRC_NAME) + "\r\n";
    send_message(client->first, message);
}

bool Server::handle_client_input(ClientIterator& client) {

    std::vector< char > buffer(512);
    ssize_t bytes_read = recv(client->first, buffer.data(), buffer.size(), MSG_DONTWAIT);

    if (bytes_read <= 0) {
        drop_client(client);
        return false;
    }

    if (client->second.input.size() > DATA_MAX) {
        drop_client(client);
        return false;
    }

    client->second.input.append(buffer.data(), bytes_read);

    for (std::string::iterator it = client->second.input.begin(); it != client->second.input.end(); it++) {
        if (*it >= '\x01' && *it <= '\x07') {
            client->second.input.erase(it);
            it--;
        }
    }

    CommandResult result;
    while (!client->second.input.empty()) {
        std::cout << "[debug][server]: input from client :" << client->first << "(" << client->second.input << ")\n";
        if (client->second.process_input(result)) {
            if (result.type == CMD_RESULT_REPLY)
                send_message(client->first, result.reply);
            else if (result.type == CMD_CHANNEL || result.type == CMD_INVITE)
                handle_channel(client, result);
            else if (result.type == CMD_QUIT) {
                drop_client(client);
                return false;
            } else if (result.type == CMD_LEAVE) {
                leave_channel(client, result);
            } else if (result.type == CMD_CHECK_USER || result.type == CMD_CHECK_PASS) {
                check_infos(client, result);
            } else if (result.type == CMD_CHANGE_MODE) {
                change_mode(client, result);
            } else if (result.type == CMD_KICK) {
                check_kick(client, result);
            } else if (result.type == CMD_TOPIC) {
                topic_handle(client, result);
            } else if (result.type == CMD_CHANGE_NICK) {
                handle_nick(client, result);
            } else if (result.type == CMD_PING) {
                ping_user(client, result);
            } else
                send_to_all(client, result);
        } else {
            break;
        }
    }
    return true;
}
