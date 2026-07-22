/* ************************************************************************** */
/*                                                                            */
/*   Client.hpp                                                               */
/*                                                                            */
/*   Phase 3: Client Connection Management                                    */
/*   Class declaration for a single connected IRC client                      */
/*                                                                            */
/*   Owner: Ismail                                                            */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>

/**
 * @class Client
 * @brief Represents one connected IRC client.
 *
 * Phase 3 scope:
 *  - Stores the client's socket file descriptor.
 *  - Stores the client's IP address (set once on creation).
 *  - Tracks whether the client has completed registration.
 *
 * This class does NOT handle:
 *  - Networking (recv/send)         → Ayoub
 *  - Parsing / Buffers              → Souhail
 *  - IRC commands (PASS/NICK/USER)  → later phases
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
		int					getFd() const;
		const std::string&	getIp() const;
		bool				isRegistered() const;

		/* --- Setters --- */
		void				setRegistered(bool state);

	private:
		/* --- Orthodox Canonical Form (disabled) --- */
		Client();
		Client(const Client& other);
		Client& operator=(const Client& other);

		/* --- Attributes --- */
		int			_fd;
		std::string	_ip;
		bool		_registered;
};

#endif
