/* ************************************************************************** */
/*                                                                            */
/*   Client.hpp                                                               */
/*                                                                            */
/*   Phase 3: Client Connection Management                                    */
/*   Phase 5: Command Dispatcher & Registration (nick/user/realname fields)   */
/*   Class declaration for a single connected IRC client                      */
/*                                                                            */
/*   Owner: Ismail                                                            */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include "BufferManager.hpp"

/**
 * @class Client
 * @brief Represents one connected IRC client.
 *
 * Phase 3 scope:
 *  - Stores the client's socket file descriptor.
 *  - Stores the client's IP address (set once on creation).
 *  - Tracks whether the client has completed registration.
 *  - Contains the client's BufferManager for storing fragmented I/O.
 *
 * Phase 5 additions:
 *  - Nickname, username, realname (set by NICK/USER commands).
 *  - passReceived flag (set after a correct PASS).
 *
 * This class does NOT handle:
 *  - Networking (recv/send)         → Ayoub
 *  - Parsing / Buffers              → Souhail
 *  - Channels / Operator status     → later phases
 *
 * Orthodox Canonical Form:
 *  - Parameterized constructor (fd + ip)
 *  - Destructor
 *  - Copy constructor        (private — disabled)
 *  - Assignment operator      (private — disabled)
 */
class Client
{
	public:
		/* --- Constructor / Destructor --- */
		Client(int fd, const std::string& ip);
		~Client();

		/* --- Getters --- */
		int					getFd()       const;
		const std::string&	getIp()       const;
		bool				isRegistered() const;
		BufferManager&		getBuffer();
		const BufferManager& getBuffer()  const;

		/* --- Phase 5: Registration fields --- */
		const std::string&	getNickname()     const;
		const std::string&	getUsername()     const;
		const std::string&	getRealname()     const;
		bool				hasReceivedPass() const;
		bool				hasNick()         const;
		bool				hasUser()         const;

		/* --- Setters --- */
		void	setRegistered(bool state);

		/* --- Phase 5: Registration setters --- */
		void	setNickname(const std::string& nick);
		void	setUsername(const std::string& user);
		void	setRealname(const std::string& real);
		void	setPassReceived(bool state);

	private:
		/* --- Orthodox Canonical Form (disabled) --- */
		Client();
		Client(const Client& other);
		Client& operator=(const Client& other);

		/* --- Attributes --- */
		int				_fd;
		std::string		_ip;
		bool			_registered;
		BufferManager	_buffer;

		/* --- Phase 5: Registration state --- */
		std::string		_nickname;		/* empty until NICK received    */
		std::string		_username;		/* empty until USER received    */
		std::string		_realname;		/* empty until USER received    */
		bool			_passReceived;	/* true after valid PASS        */
};

#endif
