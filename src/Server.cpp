#include "Server.hpp"

Server::Server(std::string port, const std::string& password) : _port(port), _password(password) {}

Server::~Server() {}

std::vector< struct pollfd >::iterator Server::addClient() {

    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;
    struct pollfd newClient;

    std::cout << "Something happened!" << std::endl;
    addrlen = sizeof(remoteaddr);
    newClient.fd = accept(this->_listener_fd, (struct sockaddr*)&remoteaddr, &addrlen);
    newClient.events = POLLIN | POLLOUT;
    newClient.revents = 0;
    if (newClient.fd == -1) {
        std::cerr << "Err" << std::endl;
    } else {
        std::cout << "New client connected with fd: " << newClient.fd << std::endl;
        this->_clients.push_back(newClient);
    }
    return (this->_clients.begin());
}

std::vector< struct pollfd >::iterator Server::deleteClient(std::vector< struct pollfd >::iterator it) {
    close(it->fd);
    it = this->_clients.erase(it);
    std::cout << "Client disconnected !" << std::endl;
    return it;
}

void Server::open_socket() {
    int yes = 1;
    int rv;
    struct addrinfo hints, *ai, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, this->_port.c_str(), &hints, &ai)) != 0) {
        throw std::exception();
    }

    for (p = ai; p != NULL; p = p->ai_next) {
        this->_listener_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (this->_listener_fd < 0) {
            continue;
        }
        setsockopt(this->_listener_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if (bind(this->_listener_fd, p->ai_addr, p->ai_addrlen) < 0) {
            close(this->_listener_fd);
            continue;
        }
        break;
    }
    freeaddrinfo(ai);
    if (p == NULL) {
        throw std::exception();
    }
    if (listen(this->_listener_fd, 10) == -1) {
        throw std::exception();
    }
}

bool Server::pollClients() {
    int poll_count = poll(this->_clients.data(), this->_clients.size(), -1);
    if (poll_count < 0) {
        std::cerr << "Poll Failed !" << std::endl;
        return false;
    }
    if (poll_count == 0) {
        std::cout << "No data changes !" << std::endl;
        return false;
    }
    return true;
}

void Server::checkClientsData() {
    char buf[512];
    for (std::vector< struct pollfd >::iterator it = _clients.begin(); it != _clients.end();) {
        if (it->revents & (POLLIN | POLLHUP)) {
            if (it->fd == this->_listener_fd) {
                it = this->addClient();
            } else {
                int nbytes = recv(it->fd, buf, sizeof buf, 0);
                if (DEBUG) {
                    std::cout << "New Message ! " << std::endl;
                    std::cout << "Bytes read: " << nbytes << std::endl;
                    std::cout << "Fd: " << it->fd << std::endl;
                }
                if (nbytes > 0) {
                    std::cout << "Something happened n2!" << std::endl;
                    std::cout << nbytes << std::endl;
                    for (std::vector< struct pollfd >::iterator ti = _clients.begin(); ti != _clients.end(); ti++) {
                        if (ti != it && ti->fd != this->_listener_fd)
                            send(ti->fd, buf, nbytes, 0);
                    }
                    write(1, buf, nbytes);
                } else if (!nbytes) {
                    it = this->deleteClient(it);
                    continue;
                }
            }
        }
        it++;
    }
}

void Server::run() {
    struct pollfd p;

    try {
        this->open_socket();
    } catch (std::exception& e) {
        std::cout << "Big err" << std::endl;
        return;
    }
    p.fd = this->_listener_fd;
    p.events = POLLIN;
    p.revents = 0;
    this->_clients.push_back(p);
    for (;;) {
        if (!this->pollClients())
            continue;
        this->checkClientsData();
    }
}
