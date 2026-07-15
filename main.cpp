#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>
#include "Server.hpp"

bool isValidPort(const std::string& portStr) {
    if (portStr.empty()) return false;
    for (size_t i = 0; i < portStr.length(); ++i) {
        if (!std::isdigit(portStr[i])) {
            return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }

    std::string portStr = argv[1];
    std::string password = argv[2];

    if (!isValidPort(portStr)) {
        std::cerr << "Error: Port khass ykun fiha ghir l-arqam (digits only)." << std::endl;
        return 1;
    }

    int port = std::atoi(portStr.c_str());
    if (port < 1024 || port > 65535) {
        std::cerr << "Error: Port khass ykun bin 1024 w 65535." << std::endl;
        return 1;
    }

    if (password.empty()) {
        std::cerr << "Error: Password maykunch khawi." << std::endl;
        return 1;
    }

    std::cout << "Server parameters verified successfully!" << std::endl;

    try {
        Server server(port, password);
        server.init();
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

