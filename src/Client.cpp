#include "Client.hpp"

Client::Client(int fd, const std::string& ip)
	: _fd(fd), _ip(ip), _registered(false),
	  _nickname(""), _username(""), _realname(""), _passReceived(false)
{
}

Client::~Client()
{
}

int	Client::getFd() const
{
	return _fd;
}

const std::string&	Client::getIp() const
{
	return _ip;
}

bool	Client::isRegistered() const
{
	return _registered;
}

BufferManager&	Client::getBuffer()
{
	return _buffer;
}

const BufferManager&	Client::getBuffer() const
{
	return _buffer;
}

const std::string&	Client::getNickname() const
{
	return _nickname;
}

const std::string&	Client::getUsername() const
{
	return _username;
}

const std::string&	Client::getRealname() const
{
	return _realname;
}

bool	Client::hasReceivedPass() const
{
	return _passReceived;
}

bool	Client::hasNick() const
{
	return !_nickname.empty();
}

bool	Client::hasUser() const
{
	return !_username.empty();
}

void	Client::setRegistered(bool state)
{
	_registered = state;
}


void	Client::setNickname(const std::string& nick)
{
	_nickname = nick;
}

void	Client::setUsername(const std::string& user)
{
	_username = user;
}

void	Client::setRealname(const std::string& real)
{
	_realname = real;
}

void	Client::setPassReceived(bool state)
{
	_passReceived = state;
}
