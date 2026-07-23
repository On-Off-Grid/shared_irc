/* ************************************************************************** */
/*                                                                            */
/*   Server.hpp                                                               */
/*                                                                            */
/*   Phase 2: Sockets & Connections                                           */
/*   Phase 3: Client Connection Management                                    */
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
# include <stdexcept>
# include <map>
# include <sys/socket.h>
# include <netinet/in.h>
# include <fcntl.h>
# include <unistd.h>

# include "Client.hpp"

class Server
{
	public:
		/* --- Orthodox Canonical Form --- */
		Server(int port, const std::string& password);
		~Server();

		/* --- Core Methods --- */
		void	init();
		void	run();

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

		/* --- Attributes --- */
		int						_port;
		std::string				_password;
		int						_serverFd;
		std::map<int, Client*>	_clients;
};

#endif
