/* ************************************************************************** */
/*                                                                            */
/*   Server.hpp                                                               */
/*                                                                            */
/*   Phase 1: Project Skeleton                                                */
/*   Class declaration for the IRC Server                                     */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <iostream>

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

		/* --- Attributes --- */
		int			_port;
		std::string	_password;
		int			_serverFd;
};

#endif
