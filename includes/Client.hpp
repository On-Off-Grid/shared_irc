#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include "BufferManager.hpp"

class Client
{
	public:
		Client(int fd, const std::string& ip);
		~Client();

		int					getFd()       const;
		const std::string&	getIp()       const;
		bool				isRegistered() const;
		BufferManager&		getBuffer();
		const BufferManager& getBuffer()  const;

		const std::string&	getNickname()     const;
		const std::string&	getUsername()     const;
		const std::string&	getRealname()     const;
		bool				hasReceivedPass() const;
		bool				hasNick()         const;
		bool				hasUser()         const;

		void	setRegistered(bool state);

		void	setNickname(const std::string& nick);
		void	setUsername(const std::string& user);
		void	setRealname(const std::string& real);
		void	setPassReceived(bool state);

	private:
		Client();
		Client(const Client& other);
		Client& operator=(const Client& other);

		int				_fd;
		std::string		_ip;
		bool			_registered;
		BufferManager	_buffer;

		std::string		_nickname;
		std::string		_username;
		std::string		_realname;
		bool			_passReceived;
};

#endif
