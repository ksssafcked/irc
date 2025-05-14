#include "Server.hpp"

Server::Server(std::string port, const std::string& password) : _port(port), _password(password), _user_nb(1) {}

Server::~Server() {}

/*void Server::add_to_pfds(struct pollfd* pfds[], int newfd, int* fd_count, int* fd_size) {
    // If we don't have room, add more space in the pfds array
    if (*fd_count == *fd_size) {
        *fd_size *= 2; // Double it

        *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN; // Check ready-to-read
    (*pfds)[*fd_count].revents = 0;

    (*fd_count)++;
}*/

std::vector< struct pollfd >::iterator Server::deleteClient(std::vector< struct pollfd >::iterator it) {
    close(it->fd);
    it = this->_clients.erase(it);
    this->_user_nb--;
    return it;
}

int Server::open_socket() {
    int listener; // Listening socket descriptor
    int yes = 1;  // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, this->_port.c_str(), &hints, &ai)) != 0) {
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // If we got here, it means we didn't get bound
    if (p == NULL) {
        return -1;
    }
    freeaddrinfo(ai); // All done with this

    // Listen
    if (listen(listener, 10) == -1) {
        return -1;
    }

    return listener;
}

void Server::run() {

    struct sockaddr_storage remoteaddr; // Client address
    socklen_t addrlen;
    int listener = this->open_socket();
    struct pollfd p;
    if (listener == -1)
        std::cout << "Big err" << std::endl;
    else {
        std::cout << "Socket opened" << std::endl;
    }
    p.fd = listener;
    p.events = POLLIN;
    p.revents = 0;
    this->_clients.push_back(p);
    for (;;) {
        int poll_count = poll(this->_clients.data(), this->_clients.size(), -1);
        std::cout << "Poll:" << poll_count << std::endl;
        if (poll_count < 0) {
            std::cerr << "Poll Failed !" << std::endl;
            continue;
        }
        if (poll_count == 0) {
            std::cout << "No data changes !" << std::endl;
            continue;
        }
        char buf[512];
        for (std::vector< struct pollfd >::iterator it = _clients.begin(); it != _clients.end(); it++) {
            if (it->revents & (POLLIN | POLLHUP)) {
                if (it->fd == listener) {

                    std::cout << "Something happened!" << std::endl;
                    addrlen = sizeof(remoteaddr);
                    struct pollfd po;
                    po.fd = accept(listener, (struct sockaddr*)&remoteaddr, &addrlen);
                    po.events = POLLIN;
                    po.revents = 0;
                    if (po.fd == -1) {
                        std::cerr << "Err" << std::endl;
                    } else {
                        _clients.push_back(po);
                        it = _clients.begin();
                        std::cout << "New client connected with fd: " << po.fd << std::endl;
                        continue;
                    }
                } else {
                    std::cout << "New Message ! " << std::endl;
                    int nbytes = recv(it->fd, buf, sizeof buf, 0);
                    std::cout << "Bytes read: " << nbytes << std::endl;
                    if (nbytes > 0) {
                        std::cout << "Something happened n2!" << std::endl;
                        std::cout << nbytes << std::endl;
                        for (std::vector< struct pollfd >::iterator ti = _clients.begin(); ti != _clients.end(); ti++) {
                            if (ti != it && ti->fd != listener)
                                send(ti->fd, buf, nbytes, 0);
                        }
                        write(1, buf, nbytes);
                    } else if (!nbytes) {
                        it = this->deleteClient(it);
                        std::cout << "End of Message" << std::endl;
                        continue;
                    }
                }
            }
        }
    }
}
