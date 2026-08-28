/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ismail <ismail@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by ss                #+#    #+#             */
/*   Updated: 2026/07/28 01:34:47 by ismail           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Channel.hpp"
#include "Parser.hpp"
#include "Server.hpp"
#include "CommandDispatcher.hpp"

/* ---- Static member definition (Phase 4: Signal Handling) ---- */
volatile sig_atomic_t Server::_shutdown = 0;

/* ************************************************************************** */
/*                          CONSTRUCTORS / DESTRUCTOR                         */
/* ************************************************************************** */

Server::Server(int port, const std::string& password)
	: _port(port), _password(password), _serverFd(-1), _dispatcher(NULL)
{
	std::cout << "Server object created. Port: " << _port
			  << ", Password: [PROTECTED]" << std::endl;
	_dispatcher = new CommandDispatcher(*this);
}

Server::~Server()
{
	delete _dispatcher;
	_dispatcher = NULL;
	_outgoingBuffers.clear();
	_clearClients();
	for (std::map<std::string, Channel*>::iterator it = _channels.begin();
		 it != _channels.end(); ++it)
	{
		delete it->second;
	}
	_channels.clear();
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
	_addPollFd(_serverFd);
	_setupSignals();
	std::cout << "Server ready." << std::endl;
}

/**
 * Phase 4: Event Loop (Ayoub)
 *
 * Replaces the Phase 2 placeholder pause() loop with a real poll()-based
 * event loop.  The flow is:
 *
 *   1. poll() blocks until at least one fd is ready.
 *   2. If the server socket is readable → accept a new client.
 *   3. If a client socket is readable   → recv() incoming bytes.
 *   4. If a client disconnects          → close + removeClient.
 *
 * No IRC parsing happens here.  Raw bytes are forwarded to BufferManager.
 */
void	Server::run()
{
	std::cout << "Server event loop started. (press Ctrl+C to stop)"
			  << std::endl;

	while (!_shutdown)
	{
		/* ---- poll(): wait for events on all monitored fds ---- */
		int pollCount = poll(&_pollfds[0], _pollfds.size(), -1);
		if (pollCount == -1)
		{
			/* EINTR: poll() was interrupted by a signal (e.g. SIGINT). */
			/* This is normal and not an error — just retry.            */
			if (errno == EINTR)
			{
				if (_shutdown)
					break ;
				continue ;
			}
			throw std::runtime_error(
				std::string("poll() failed: ") + strerror(errno));
		}

		/* ---- Iterate over all monitored fds ---- */
		for (size_t i = 0; i < _pollfds.size(); ++i)
		{
			if (_pollfds[i].fd == -1)
				continue ;

			/* Skip fds with no pending events */
			if (_pollfds[i].revents == 0)
				continue ;

			/* ---- Error / hang-up on this fd ---- */
			if (_pollfds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
			{
				if (_pollfds[i].fd == _serverFd)
				{
					throw std::runtime_error(
						"poll() reported error on server socket");
				}
				disconnectClient(_pollfds[i].fd);
				continue ;
			}

			/* ---- Readable event (POLLIN) ---- */
			if (_pollfds[i].revents & POLLIN)
			{
				if (_pollfds[i].fd == _serverFd)
				{
					/* New incoming connection */
					_acceptNewClient();
				}
				else
				{
					/* Data from an existing client */
					_receiveData(_pollfds[i].fd);
				}
			}

			/* ---- Writable event (POLLOUT) ---- */
			if (_pollfds[i].fd != -1 && _pollfds[i].fd != _serverFd
				&& (_pollfds[i].revents & POLLOUT))
			{
				_flushOutgoingData(_pollfds[i].fd);
			}
		}

		/* ---- Clean up disconnected fds from _pollfds (post-loop) ---- */
		for (size_t i = 0; i < _pollfds.size(); )
		{
			if (_pollfds[i].fd == -1)
			{
				_pollfds[i] = _pollfds[_pollfds.size() - 1];
				_pollfds.pop_back();
			}
			else
			{
				++i;
			}
		}
	}

	std::cout << "\nServer shutting down gracefully." << std::endl;

	/* ---- Close every connected client socket ---- */
	for (std::map<int, Client*>::iterator it = _clients.begin();
		 it != _clients.end(); ++it)
	{
		std::cout << "Closing client socket (fd " << it->first << ")."
				  << std::endl;
		close(it->first);
	}

	/* ---- Remove every client (frees memory) ---- */
	_clearClients();
	for (std::map<std::string, Channel*>::iterator it = _channels.begin();
		 it != _channels.end(); ++it)
	{
		delete it->second;
	}
	_channels.clear();
	_outgoingBuffers.clear();
	_pollfds.clear();

	/* ---- Close the listening socket ---- */
	if (_serverFd != -1)
	{
		std::cout << "Closing server socket (fd " << _serverFd << ")."
				  << std::endl;
		close(_serverFd);
		_serverFd = -1;
	}

	std::cout << "Server shutdown complete." << std::endl;
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

/* ************************************************************************** */
/*                    EVENT LOOP HELPERS (Phase 4 — Ayoub)                    */
/* ************************************************************************** */

/**
 * @brief Accept a new incoming TCP connection.
 *
 * Flow:
 *   1. accept() returns a new client fd and fills the address struct.
 *   2. Convert the IP to a printable string with inet_ntoa().
 *   3. Set the new client socket to non-blocking.
 *   4. Register the client: server.addClient(clientFd, clientIp).
 *   5. Monitor the new fd by adding it to the pollfd vector.
 *
 * accept() is called in a loop to drain all pending connections,
 * because the server socket is non-blocking and poll() only wakes us once.
 */
void	Server::_acceptNewClient()
{
	while (true)
	{
		struct sockaddr_in clientAddr;
		socklen_t addrLen = sizeof(clientAddr);
		std::memset(&clientAddr, 0, sizeof(clientAddr));

		int clientFd = accept(_serverFd,
			(struct sockaddr *)&clientAddr, &addrLen);
		if (clientFd == -1)
		{
			/* EAGAIN / EWOULDBLOCK: no more pending connections.        */
			/* This is expected on a non-blocking socket — just return.  */
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break ;
			/* EINTR: accept was interrupted by a signal — retry.        */
			if (errno == EINTR)
				continue ;
			/* Any other error is unexpected.                            */
			std::cerr << "accept() failed: " << strerror(errno) << std::endl;
			break ;
		}

		/* Convert binary IP to human-readable string */
		std::string clientIp(inet_ntoa(clientAddr.sin_addr));

		/* Set the new client socket to non-blocking */
		if (!_setSocketNonBlocking(clientFd))
			continue ;

		/* Register client in the Server's client map */
		addClient(clientFd, clientIp);

		/* Start monitoring this fd for POLLIN events */
		_addPollFd(clientFd);

		std::cout << "New connection from " << clientIp
				  << " (fd " << clientFd << ")." << std::endl;
	}
}

/**
 * @brief Receive raw bytes from a connected client.
 *
 * @param fd  The file descriptor to recv() from.
 *
 * Flow:
 *   1. recv() into a local buffer.
 *   2. If recv() returns 0 → graceful disconnect (client closed connection).
 *   3. If recv() returns -1 → check errno.
 *      - EAGAIN/EWOULDBLOCK: nothing to read right now (normal).
 *      - EINTR: interrupted by signal, retry.
 *      - anything else: fatal error → disconnect.
 *   4. If bytes > 0 → forward raw data to Client's BufferManager.
 *
 * This function does NOT parse or interpret the received data.
 * It simply calls client->getBuffer().append_data(rawBytes).
 */
void	Server::_receiveData(int fd)
{
	char buf[1024];

	ssize_t bytesRead = recv(fd, buf, sizeof(buf), 0);

	/* ---- Graceful disconnect ---- */
	if (bytesRead == 0)
	{
		std::cout << "Client disconnected (fd " << fd << ")." << std::endl;
		disconnectClient(fd);
		return ;
	}

	/* ---- Error handling ---- */
	if (bytesRead == -1)
	{
		/* Non-blocking socket: nothing to read right now. */
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return ;
		/* Signal interrupted the call — not an error. */
		if (errno == EINTR)
			return ;
		/* Fatal recv error — disconnect this client. */
		std::cerr << "recv() error on fd " << fd << ": "
				  << strerror(errno) << std::endl;
		disconnectClient(fd);
		return ;
	}

	/* ---- Forward raw bytes to the client's BufferManager ---- */
	Client* client = getClient(fd);
	if (client)
	{
		std::string rawData(buf, bytesRead);
		client->getBuffer().append_data(rawData);

		/* Extract and process any complete lines (Phase 4 hook) */
		_processClientBuffer(fd);
	}
}

/**
 * @brief Cleanly disconnect a client.
 *
 * @param fd  The file descriptor of the client to disconnect.
 *
 * Ownership rules:
 *   - Networking layer owns: socket descriptors, pollfd collection.
 *   - Server owns: Client objects.
 *
 * So we:
 *   1. close(fd)        — networking layer responsibility.
 *   2. removeClient(fd) — delegates to Server to delete Client*.
 *   3. _removePollFd(fd) — stop monitoring this fd.
 *
 * Now public (Phase 5) so CommandDispatcher can call it on QUIT.
 */
void	Server::disconnectClient(int fd)
{
	Client* client = getClient(fd);
	if (client)
	{
		std::string nick = client->getNickname().empty() ? "*" : client->getNickname();
		std::string user = client->getUsername().empty() ? "*" : client->getUsername();
		std::string quitMsg = ":" + nick + "!" + user + "@" + client->getIp() + " QUIT :Client disconnected";

		std::vector<std::string> emptyChannels;
		for (std::map<std::string, Channel*>::iterator it = _channels.begin();
		     it != _channels.end(); ++it)
		{
			Channel* chan = it->second;
			if (chan->hasMember(fd))
			{
				broadcastToChannel(*chan, quitMsg, fd);
				chan->removeMember(fd);
			}
			chan->removeInvite(fd);
			if (chan->getMembers().empty())
			{
				emptyChannels.push_back(chan->getName());
			}
		}
		for (size_t i = 0; i < emptyChannels.size(); ++i)
		{
			deleteChannel(emptyChannels[i]);
		}
	}

	close(fd);
	_outgoingBuffers.erase(fd);
	removeClient(fd);
	_removePollFd(fd);
}

/**
 * @brief Return the server password (Phase 5).
 *
 * Used by CommandDispatcher to validate the PASS command.
 */
const std::string&	Server::getPassword() const
{
	return _password;
}

/**
 * @brief Add a file descriptor to the pollfd collection.
 *
 * @param fd  The fd to monitor.
 *
 * All fds are monitored for POLLIN (data available to read).
 * POLLERR and POLLHUP are always reported by poll() regardless
 * of the events field, so we don't need to set them explicitly.
 */
void	Server::_addPollFd(int fd)
{
	struct pollfd pfd;
	std::memset(&pfd, 0, sizeof(pfd));
	pfd.fd      = fd;
	pfd.events  = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);
}

/**
 * @brief Remove a file descriptor from the pollfd collection.
 *
 * @param fd  The fd to stop monitoring.
 *
 * We find the entry by linear scan and swap-erase for O(1) removal.
 * The order of pollfd entries does not matter to poll().
 */
void	Server::_removePollFd(int fd)
{
	for (size_t i = 0; i < _pollfds.size(); ++i)
	{
		if (_pollfds[i].fd == fd)
		{
			// Mark for deferred cleanup in Server::run() to avoid index mutation mid-iteration.
			_pollfds[i].fd = -1;
			_pollfds[i].revents = 0;
			return ;
		}
	}
}

void	Server::_setPollOut(int fd, bool enabled)
{
	for (size_t i = 0; i < _pollfds.size(); ++i)
	{
		if (_pollfds[i].fd == fd)
		{
			if (enabled)
				_pollfds[i].events |= POLLOUT;
			else
				_pollfds[i].events &= ~POLLOUT;
			return ;
		}
	}
}

/**
 * @brief Set a socket to non-blocking mode.
 *
 * @param fd  The socket file descriptor.
 *
 * Uses fcntl() to add O_NONBLOCK to the existing flags.
 * If this fails, the socket is closed and the client is not added.
 *
 * This is separate from _setNonBlocking() which handles the server socket
 * and throws on error. For client sockets, we log and close gracefully.
 */
bool	Server::_setSocketNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
	{
		std::cerr << "fcntl(F_GETFL) failed on fd " << fd << ": "
				  << strerror(errno) << std::endl;
		close(fd);
		return false;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		std::cerr << "fcntl(F_SETFL) failed on fd " << fd << ": "
				  << strerror(errno) << std::endl;
		close(fd);
		return false;
	}
	return true;
}

/* ************************************************************************** */
/*                SEND / BUFFER-PROCESS (Phase 4 — Ayoub)                     */
/* ************************************************************************** */

/**
 * @brief Extract complete lines from a client's BufferManager and dispatch them.
 *
 * @param fd  The file descriptor of the client.
 *
 * Phase 5 replaces the Phase 4 TODO: instead of logging only, each valid
 * ParsedCommand is forwarded to CommandDispatcher::dispatch(), which routes
 * it to the correct handler (PASS, NICK, USER, QUIT, …).
 *
 * The dispatcher may call disconnectClient() if it processes a QUIT, so
 * we check hasClient(fd) after every dispatch() call.
 */
void	Server::_processClientBuffer(int fd)
{
	Client* client = getClient(fd);
	if (!client)
		return ;

	std::vector<std::string> lines = client->getBuffer().extract_complete_lines();
	for (size_t j = 0; j < lines.size(); ++j)
	{
		/* Safety: dispatcher may have disconnected this client */
		if (!hasClient(fd))
			break ;

		std::cout << "[fd=" << fd << "] Raw: " << lines[j] << std::endl;

		ParsedCommand cmd = _parser.parse_line(lines[j]);
		if (cmd.is_valid)
			_dispatcher->dispatch(fd, cmd);
		else
			std::cout << "[fd=" << fd << "] (invalid/empty line)" << std::endl;
	}
}

/**
 * @brief Flush queued bytes to a socket after poll() reports POLLOUT.
 *
 * @param fd    The file descriptor to write to.
 * send() is only called from this POLLOUT path, which keeps outgoing I/O
 * under the same poll loop as incoming data and the listening socket.
 */
void	Server::_flushOutgoingData(int fd)
{
	std::map<int, std::string>::iterator it = _outgoingBuffers.find(fd);
	if (it == _outgoingBuffers.end())
	{
		_setPollOut(fd, false);
		return ;
	}

	std::string& data = it->second;
	while (!data.empty())
	{
		ssize_t bytesSent = send(fd, data.c_str(), data.size(), 0);
		if (bytesSent == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return ;
			if (errno == EINTR)
				continue ;
			std::cerr << "send() error on fd " << fd << ": "
					  << strerror(errno) << std::endl;
			disconnectClient(fd);
			return ;
		}
		if (bytesSent == 0)
			return ;
		data.erase(0, bytesSent);
	}
	_outgoingBuffers.erase(fd);
	_setPollOut(fd, false);
}

/**
 * @brief Public API to send an IRC message to a specific client.
 *
 * @param fd       The client's file descriptor.
 * @param message  The IRC message to send (without trailing \r\n).
 *
 * Automatically appends \r\n per the IRC protocol (RFC 2812 §2.3), queues
 * the bytes, then enables POLLOUT so the event loop can flush them.
 */
void	Server::sendToClient(int fd, const std::string& message)
{
	if (!hasClient(fd))
		return ;
	_outgoingBuffers[fd] += message + "\r\n";
	_setPollOut(fd, true);
}

void	Server::broadcastToChannel(const Channel& channel,
		const std::string& message, int excludeFd)
{
	const std::map<int, Client*>& members = channel.getMembers();
	for (std::map<int, Client*>::const_iterator it = members.begin();
		 it != members.end(); ++it)
	{
		if (it->first != excludeFd)
			sendToClient(it->first, message);
	}
}

/* ************************************************************************** */
/*                SIGNAL HANDLING (Phase 4 — Ayoub)                           */
/* ************************************************************************** */

/**
 * @brief Static signal handler for SIGINT and SIGTERM.
 *
 * @param signum  The signal number received.
 *
 * Sets the _shutdown flag to 1.  The event loop in run() checks
 * this flag and exits cleanly, allowing the destructor to free
 * all resources (close fds, delete clients).
 *
 * Note: Only async-signal-safe operations are allowed here.
 * Setting a volatile sig_atomic_t variable is safe.
 */
void	Server::_signalHandler(int signum)
{
	(void)signum;
	_shutdown = 1;
}

/**
 * @brief Register signal handlers for graceful shutdown.
 *
 * Catches SIGINT (Ctrl+C) and SIGTERM so the server can clean up
 * instead of dying abruptly.  Uses sigaction() for portable,
 * reliable signal handling (preferred over signal()).
 */
void	Server::_setupSignals()
{
	struct sigaction sa;
	std::memset(&sa, 0, sizeof(sa));
	sa.sa_handler = _signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if (sigaction(SIGINT, &sa, NULL) == -1)
		throw std::runtime_error("sigaction(SIGINT) failed");
	if (sigaction(SIGTERM, &sa, NULL) == -1)
		throw std::runtime_error("sigaction(SIGTERM) failed");
	sa.sa_handler = SIG_IGN;
	if (sigaction(SIGPIPE, &sa, NULL) == -1)
		throw std::runtime_error("sigaction(SIGPIPE) failed");
}

/* ************************************************************************** */
/*                    CHANNEL MANAGEMENT (Phase 6)                            */
/* ************************************************************************** */

Channel* Server::getChannel(const std::string& name) const
{
	std::map<std::string, Channel*>::const_iterator it = _channels.find(name);
	if (it == _channels.end())
		return NULL;
	return it->second;
}

Channel* Server::createChannel(const std::string& name)
{
	Channel* chan = getChannel(name);
	if (chan)
		return chan;
	chan = new Channel(name);
	try
	{
		_channels[name] = chan;
	}
	catch (...)
	{
		delete chan;
		throw;
	}
	std::cout << "Channel created: " << name << std::endl;
	return chan;
}

void Server::deleteChannel(const std::string& name)
{
	std::map<std::string, Channel*>::iterator it = _channels.find(name);
	if (it == _channels.end())
		return;
	delete it->second;
	_channels.erase(it);
	std::cout << "Channel deleted: " << name << std::endl;
}

const std::map<std::string, Channel*>& Server::getChannels() const
{
	return _channels;
}

Client* Server::getClientByNickname(const std::string& nick) const
{
	for (std::map<int, Client*>::const_iterator it = _clients.begin();
	     it != _clients.end(); ++it)
	{
		std::string n1 = it->second->getNickname();
		std::string n2 = nick;
		if (n1.size() == n2.size())
		{
			bool equal = true;
			for (size_t i = 0; i < n1.size(); ++i)
			{
				char c1 = n1[i];
				char c2 = n2[i];
				if (c1 >= 'A' && c1 <= 'Z') c1 = c1 - 'A' + 'a';
				if (c2 >= 'A' && c2 <= 'Z') c2 = c2 - 'A' + 'a';
				if (c1 == '[') c1 = '{';
				else if (c1 == ']') c1 = '}';
				else if (c1 == '\\') c1 = '|';
				else if (c1 == '^') c1 = '~';

				if (c2 == '[') c2 = '{';
				else if (c2 == ']') c2 = '}';
				else if (c2 == '\\') c2 = '|';
				else if (c2 == '^') c2 = '~';

				if (c1 != c2)
				{
					equal = false;
					break;
				}
			}
			if (equal)
				return it->second;
		}
	}
	return NULL;
}
