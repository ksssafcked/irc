#include "Server.hpp"

Server::Server(unsigned short port, const std::string& password) : _port(port), _password(password) {}

Server::~Server() {}

void Server::run() {}
