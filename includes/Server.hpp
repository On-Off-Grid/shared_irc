#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>

class Server {
private:
    int         _server_fd;
    int         _port;
    std::string _password;

    // Orthodox Canonical Form: Prevent copying
    Server(const Server& other);
    Server& operator=(const Server& other);

public:
    Server(int port, const std::string& password);
    ~Server();

    void init();
    void run();
};

#endif
