#include "Server.hpp"
#include <iostream>

Server::Server(int port, const std::string& password)
    : _server_fd(-1), _port(port), _password(password) {
    std::cout << "Server object created. Port: " << _port << ", Password: [PROTECTED]" << std::endl;
}

Server::~Server() {
    std::cout << "Server object destroyed." << std::endl;
}

void Server::init() {
    // Empty for now (Phase 2 - Step 1: No networking implementation)
}

void Server::run() {
    // Empty for now (Phase 2 - Step 1: No networking implementation)
}
