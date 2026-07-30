/* ************************************************************************** */
/*                                                                            */
/*   Server.hpp                                                               */
/*                                                                            */
/*   Phase 2: Sockets & Connections                                           */
/*   Phase 3: Client Connection Management                                    */
/*   Phase 4: Event Loop & Networking Layer                                   */
/*   Class declaration for the IRC Server                                     */
/*                                                                            */
/* ************************************************************************** */

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

class Server
{
	public:
		/* --- Orthodox Canonical Form --- */
		Server(int port, const std::string& password);
		~Server();

		/* --- Core Methods --- */
		void	init();
		void	run();

		/* --- Send data to a client (Phase 4 — Ayoub) --- */
		void	sendToClient(int fd, const std::string& message);

		/* --- Client Management (Phase 3 — Ismail) --- */
		void							addClient(int fd, const std::string& ip);
		void							removeClient(int fd);
		Client*							getClient(int fd) const;
		bool							hasClient(int fd) const;
		const std::map<int, Client*>&	getClients() const;

	private:
		/* --- Orthodox Canonical Form (disabled) --- */
		Server();
		Server(const Server& other);
		Server& operator=(const Server& other);

		/* --- Private Helpers (Phase 2: Networking) --- */
		void	_createSocket();
		void	_configureSocket();
		void	_setNonBlocking();
		void	_bindSocket();
		void	_startListening();

		/* --- Private Helpers (Phase 3: Client Cleanup) --- */
		void	_clearClients();

		/* --- Private Helpers (Phase 4: Event Loop — Ayoub) --- */
		void	_acceptNewClient();
		void	_receiveData(int fd);
		void	_processClientBuffer(int fd);
		void	_sendData(int fd, const std::string& data);
		void	_disconnectClient(int fd);
		void	_addPollFd(int fd);
		void	_removePollFd(int fd);
		void	_setSocketNonBlocking(int fd);

		/* --- Signal Handling (Phase 4 — Ayoub) --- */
		static void					_signalHandler(int signum);
		void						_setupSignals();
		static volatile sig_atomic_t	_shutdown;

		/* --- Attributes --- */
		int							_port;
		std::string					_password;
		int							_serverFd;
		std::map<int, Client*>		_clients;
		std::vector<struct pollfd>	_pollfds;	/* Phase 4: pollfd collection */
	Parser						_parser;	/* Phase 4: IRC line parser   */
};

#endif
