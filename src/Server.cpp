#include "Client.hpp"
#include "Channel.hpp"
#include "Parser.hpp"
#include "Server.hpp"
#include "CommandDispatcher.hpp"

volatile sig_atomic_t Server::_shutdown = 0;

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

void	Server::run()
{
	std::cout << "Server event loop started. (press Ctrl+C to stop)"
			  << std::endl;

	while (!_shutdown)
	{
		int pollCount = poll(&_pollfds[0], _pollfds.size(), -1);
		if (pollCount == -1)
		{
			if (errno == EINTR)
			{
				if (_shutdown)
					break ;
				continue ;
			}
			throw std::runtime_error(
				std::string("poll() failed: ") + strerror(errno));
		}

		for (size_t i = 0; i < _pollfds.size(); ++i)
		{
			if (_pollfds[i].fd == -1)
				continue ;

			if (_pollfds[i].revents == 0)
				continue ;

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

			if (_pollfds[i].revents & POLLIN)
			{
				if (_pollfds[i].fd == _serverFd)
				{
					_acceptNewClient();
				}
				else
				{
					_receiveData(_pollfds[i].fd);
				}
			}

			if (_pollfds[i].fd != -1 && _pollfds[i].fd != _serverFd
				&& (_pollfds[i].revents & POLLOUT))
			{
				_flushOutgoingData(_pollfds[i].fd);
			}
		}

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

	for (std::map<int, Client*>::iterator it = _clients.begin();
		 it != _clients.end(); ++it)
	{
		std::cout << "Closing client socket (fd " << it->first << ")."
				  << std::endl;
		close(it->first);
	}

	_clearClients();
	for (std::map<std::string, Channel*>::iterator it = _channels.begin();
		 it != _channels.end(); ++it)
	{
		delete it->second;
	}
	_channels.clear();
	_outgoingBuffers.clear();
	_pollfds.clear();

	if (_serverFd != -1)
	{
		std::cout << "Closing server socket (fd " << _serverFd << ")."
				  << std::endl;
		close(_serverFd);
		_serverFd = -1;
	}

	std::cout << "Server shutdown complete." << std::endl;
}

void	Server::_createSocket()
{
	std::cout << "Creating socket..." << std::endl;
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd == -1)
		throw std::runtime_error(std::string("socket() failed: ") + strerror(errno));
	std::cout << "Socket created (fd " << _serverFd << ")." << std::endl;
}

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

void	Server::_setNonBlocking()
{
	std::cout << "Enabling non-blocking mode..." << std::endl;
	if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) == -1)
	{
		close(_serverFd);
		_serverFd = -1;
		throw std::runtime_error(std::string("fcntl(F_SETFL, O_NONBLOCK) failed: ") + strerror(errno));
	}
	std::cout << "Socket set to non-blocking." << std::endl;
}

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

void	Server::removeClient(int fd)
{
	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return ;
	delete it->second;
	_clients.erase(it);
	std::cout << "Client removed (fd " << fd << ")." << std::endl;
}

Client*	Server::getClient(int fd) const
{
	std::map<int, Client*>::const_iterator it = _clients.find(fd);
	if (it == _clients.end())
		return NULL;
	return it->second;
}

bool	Server::hasClient(int fd) const
{
	return _clients.find(fd) != _clients.end();
}

const std::map<int, Client*>&	Server::getClients() const
{
	return _clients;
}

void	Server::_clearClients()
{
	for (std::map<int, Client*>::iterator it = _clients.begin();
		 it != _clients.end(); ++it)
	{
		delete it->second;
	}
	_clients.clear();
}

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
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break ;
			if (errno == EINTR)
				continue ;
			std::cerr << "accept() failed: " << strerror(errno) << std::endl;
			break ;
		}

		std::string clientIp(inet_ntoa(clientAddr.sin_addr));

		if (!_setSocketNonBlocking(clientFd))
			continue ;

		addClient(clientFd, clientIp);

		_addPollFd(clientFd);

		std::cout << "New connection from " << clientIp
				  << " (fd " << clientFd << ")." << std::endl;
	}
}

void	Server::_receiveData(int fd)
{
	char buf[1024];

	ssize_t bytesRead = recv(fd, buf, sizeof(buf), 0);

	if (bytesRead == 0)
	{
		std::cout << "Client disconnected (fd " << fd << ")." << std::endl;
		disconnectClient(fd);
		return ;
	}

	if (bytesRead == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return ;
		if (errno == EINTR)
			return ;
		std::cerr << "recv() error on fd " << fd << ": "
				  << strerror(errno) << std::endl;
		disconnectClient(fd);
		return ;
	}

	Client* client = getClient(fd);
	if (client)
	{
		std::string rawData(buf, bytesRead);
		client->getBuffer().append_data(rawData);

		_processClientBuffer(fd);
	}
}

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

const std::string&	Server::getPassword() const
{
	return _password;
}

void	Server::_addPollFd(int fd)
{
	struct pollfd pfd;
	std::memset(&pfd, 0, sizeof(pfd));
	pfd.fd      = fd;
	pfd.events  = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);
}

void	Server::_removePollFd(int fd)
{
	for (size_t i = 0; i < _pollfds.size(); ++i)
	{
		if (_pollfds[i].fd == fd)
		{
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

bool	Server::_setSocketNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "fcntl(F_SETFL, O_NONBLOCK) failed on fd " << fd << ": "
				  << strerror(errno) << std::endl;
		close(fd);
		return false;
	}
	return true;
}

void	Server::_processClientBuffer(int fd)
{
	Client* client = getClient(fd);
	if (!client)
		return ;

	std::vector<std::string> lines = client->getBuffer().extract_complete_lines();
	for (size_t j = 0; j < lines.size(); ++j)
	{
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

void	Server::_signalHandler(int signum)
{
	(void)signum;
	_shutdown = 1;
}

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
