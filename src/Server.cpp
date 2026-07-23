/* ************************************************************************** */
/*                                                                            */
/*   Server.cpp                                                               */
/*                                                                            */
/*   Phase 2: Sockets & Connections                                           */
/*   Phase 3: Client Connection Management                                    */
/*   Implementation of the Server class                                       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

/* ************************************************************************** */
/*                          CONSTRUCTORS / DESTRUCTOR                         */
/* ************************************************************************** */

Server::Server(int port, const std::string& password)
	: _port(port), _password(password), _serverFd(-1)
{
	std::cout << "Server object created. Port: " << _port
			  << ", Password: [PROTECTED]" << std::endl;
}

Server::~Server()
{
	_clearClients();
	if (_serverFd != -1)
	{
		close(_serverFd);
		std::cout << "Server socket closed (fd " << _serverFd << ")." << std::endl;
	}
	std::cout << "Server object destroyed." << std::endl;
}

/* ************************************************************************** */
/*                              CORE METHODS                                  */
/* ************************************************************************** */

void	Server::init()
{
	std::cout << "Initializing server..." << std::endl;
	_createSocket();
	_configureSocket();
	_setNonBlocking();
	_bindSocket();
	_startListening();
	std::cout << "Server ready." << std::endl;
}

void	Server::run()
{
	std::cout << "Server is waiting... (press Ctrl+C to stop)" << std::endl;
	/* Phase 2 ends after listen(). No poll(), no accept(), no recv().       */
	/* We simply pause to keep the process alive so the socket stays open.   */
	while (true)
		pause();
}

/* ************************************************************************** */
/*                          PRIVATE HELPERS                                   */
/* ************************************************************************** */

/**
 * Step 1: Create a TCP socket using socket().
 * AF_INET    — IPv4
 * SOCK_STREAM — TCP (reliable, ordered, connection-oriented)
 * 0          — let the OS pick the protocol (TCP for SOCK_STREAM)
 */
void	Server::_createSocket()
{
	std::cout << "Creating socket..." << std::endl;
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd == -1)
		throw std::runtime_error(std::string("socket() failed: ") + strerror(errno));
	std::cout << "Socket created (fd " << _serverFd << ")." << std::endl;
}

/**
 * Step 2: Set SO_REUSEADDR so we can immediately re-bind after a restart.
 * Without this, bind() would fail with "Address already in use" for a while
 * after the server exits, because the port stays in TIME_WAIT state.
 */
void	Server::_configureSocket()
{
	std::cout << "Setting SO_REUSEADDR..." << std::endl;
	int optval = 1;
	if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
	{
		close(_serverFd);
		_serverFd = -1;
		throw std::runtime_error(std::string("setsockopt() failed: ") + strerror(errno));
	}
	std::cout << "Socket configured." << std::endl;
}

/**
 * Step 3: Enable non-blocking mode with fcntl().
 * This is required so that later phases can use poll() without blocking.
 * We first GET the current flags, then SET O_NONBLOCK on top.
 */
void	Server::_setNonBlocking()
{
	std::cout << "Enabling non-blocking mode..." << std::endl;
	int flags = fcntl(_serverFd, F_GETFL, 0);
	if (flags == -1)
	{
		close(_serverFd);
		_serverFd = -1;
		throw std::runtime_error(std::string("fcntl(F_GETFL) failed: ") + strerror(errno));
	}
	if (fcntl(_serverFd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		close(_serverFd);
		_serverFd = -1;
		throw std::runtime_error(std::string("fcntl(F_SETFL) failed: ") + strerror(errno));
	}
	std::cout << "Socket set to non-blocking." << std::endl;
}

/**
 * Step 4 & 5: Prepare a sockaddr_in and bind the socket to the port.
 * AF_INET      — IPv4
 * INADDR_ANY   — accept connections on any network interface
 * htons(_port) — convert port to network byte order
 */
void	Server::_bindSocket()
{
	std::cout << "Binding to port " << _port << "..." << std::endl;

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port        = htons(_port);

	if (bind(_serverFd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		close(_serverFd);
		_serverFd = -1;
		throw std::runtime_error(std::string("bind() failed: ") + strerror(errno));
	}
	std::cout << "Bound successfully." << std::endl;
}

/**
 * Step 6: Start listening for incoming connections.
 * The backlog (10) is the maximum number of pending connections queued
 * by the kernel before they are accepted. This is a reasonable default.
 */
void	Server::_startListening()
{
	std::cout << "Listening..." << std::endl;
	if (listen(_serverFd, 10) == -1)
	{
		close(_serverFd);
		_serverFd = -1;
		throw std::runtime_error(std::string("listen() failed: ") + strerror(errno));
	}
	std::cout << "Server listening on port " << _port << std::endl;
}

/* ************************************************************************** */
/*                    CLIENT MANAGEMENT (Phase 3 — Ismail)                    */
/* ************************************************************************** */

/**
 * @brief Adds a new Client to the server.
 * @param fd  The file descriptor returned by accept().
 * @param ip  The IP address string of the remote client.
 *
 * If a client with the same fd already exists, this is a logic error
 * (caller should never add a duplicate), so we silently ignore it.
 *
 * Integration:
 *   Ayoub's networking layer will call this right after accept():
 *     int clientFd = accept(...);
 *     server.addClient(clientFd, ipString);
 */
void	Server::addClient(int fd, const std::string& ip)
{
	if (_clients.find(fd) != _clients.end())
		return ;
	Client* client = new Client(fd, ip);
	try
	{
		_clients.insert(std::map<int, Client*>::value_type(fd, client));
	}
	catch (...)
	{
		delete client;
		throw;
	}
	std::cout << "Client added (fd " << fd << ", ip " << ip << ")." << std::endl;
}

/**
 * @brief Removes and deletes a Client from the server.
 * @param fd  The file descriptor of the client to remove.
 *
 * If the fd is not found the call is a no-op, which avoids crashes
 * in edge cases (double-remove, race on disconnect, etc.).
 *
 * The caller (Ayoub’s networking layer) is responsible for calling
 * close(fd) before or after this call.
 */
void	Server::removeClient(int fd)
{
	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return ;
	delete it->second;
	_clients.erase(it);
	std::cout << "Client removed (fd " << fd << ")." << std::endl;
}

/**
 * @brief Returns a pointer to the Client with the given fd.
 * @param fd  The file descriptor to look up.
 * @return    Pointer to the Client, or NULL if not found.
 */
Client*	Server::getClient(int fd) const
{
	std::map<int, Client*>::const_iterator it = _clients.find(fd);
	if (it == _clients.end())
		return NULL;
	return it->second;
}

/**
 * @brief Checks whether a Client with the given fd exists.
 * @param fd  The file descriptor to check.
 * @return    true if found, false otherwise.
 */
bool	Server::hasClient(int fd) const
{
	return _clients.find(fd) != _clients.end();
}

/**
 * @brief Returns a const reference to the entire client map.
 *
 * Useful for Ayoub’s event loop to iterate over all connected clients
 * when building the pollfd array or broadcasting messages.
 */
const std::map<int, Client*>&	Server::getClients() const
{
	return _clients;
}

/**
 * @brief Deletes every Client and clears the container.
 *
 * Called from the destructor to guarantee no memory leaks.
 * After this call the map is empty and every Client* is freed.
 */
void	Server::_clearClients()
{
	for (std::map<int, Client*>::iterator it = _clients.begin();
		 it != _clients.end(); ++it)
	{
		delete it->second;
	}
	_clients.clear();
}
