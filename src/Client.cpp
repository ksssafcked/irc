#include "Client.hpp"

Client::Client(int fd, std::string username) : _fd(fd), _username(username) {}

Client::Client(int fd) : _fd(fd) {}
