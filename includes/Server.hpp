#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <iostream>
# include <cstring>
# include <cerrno>
# include <cstdlib>
# include <csignal>
# include <stdexcept>
# include <map>
# include "Parser.hpp"
# include <vector>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <fcntl.h>
# include <unistd.h>
# include <poll.h>

class Client;
class Channel;
class CommandDispatcher;

class Server
{
	public:
		Server(int port, const std::string& password);
		~Server();

		void	init();
		void	run();

		void	sendToClient(int fd, const std::string& message);
		void	broadcastToChannel(const Channel& channel,
					const std::string& message, int excludeFd = -1);

		void							addClient(int fd, const std::string& ip);
		void							removeClient(int fd);
		Client*							getClient(int fd) const;
		bool							hasClient(int fd) const;
		const std::map<int, Client*>&	getClients() const;

		void						disconnectClient(int fd);

		const std::string&			getPassword() const;

		Channel*								getChannel(const std::string& name) const;
		Channel*								createChannel(const std::string& name);
		void									deleteChannel(const std::string& name);
		const std::map<std::string, Channel*>&	getChannels() const;
		Client*									getClientByNickname(const std::string& nick) const;

	private:
		Server();
		Server(const Server& other);
		Server& operator=(const Server& other);

		void	_createSocket();
		void	_configureSocket();
		void	_setNonBlocking();
		void	_bindSocket();
		void	_startListening();

		void	_clearClients();

		void	_acceptNewClient();
		void	_receiveData(int fd);
		void	_processClientBuffer(int fd);
		void	_flushOutgoingData(int fd);
		void	_addPollFd(int fd);
		void	_removePollFd(int fd);
		void	_setPollOut(int fd, bool enabled);
		bool	_setSocketNonBlocking(int fd);

		static void					_signalHandler(int signum);
		void						_setupSignals();
		static volatile sig_atomic_t	_shutdown;

		int							_port;
		std::string					_password;
		int							_serverFd;
		std::map<int, Client*>		_clients;
		std::map<std::string, Channel*>	_channels;
		std::map<int, std::string>	_outgoingBuffers;
		std::vector<struct pollfd>	_pollfds;
		Parser					_parser;
		CommandDispatcher*			_dispatcher;
};

#endif
