/* ************************************************************************** */
/*                                                                            */
/*   Server.hpp                                                               */
/*                                                                            */
/*   Phase 2: Sockets & Connections                                           */
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
# include <sys/socket.h>
# include <netinet/in.h>
# include <fcntl.h>
# include <unistd.h>

class Server
{
	public:
		/* --- Orthodox Canonical Form --- */
		Server(int port, const std::string& password);
		~Server();

		/* --- Core Methods --- */
		void	init();
		void	run();

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

		/* --- Attributes --- */
		int			_port;
		std::string	_password;
		int			_serverFd;
};

#endif
