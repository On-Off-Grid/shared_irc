/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ismail <ismail@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 15:38:33 by ismail           #+#    #+#             */
/*   Updated: 2026/07/22 21:09:20 by ismail           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

/* ************************************************************************** */
/*                          CONSTRUCTORS / DESTRUCTOR                         */
/* ************************************************************************** */

/**
 * @brief Parameterized constructor.
 * @param fd  The socket file descriptor returned by accept().
 * @param ip  The IP address of the remote client (from inet_ntoa or similar).
 *
 * Registration starts as false.  It is set to true by CommandDispatcher
 * once PASS + NICK + USER have all been accepted.
 */
Client::Client(int fd, const std::string& ip)
	: _fd(fd), _ip(ip), _registered(false),
	  _nickname(""), _username(""), _realname(""), _passReceived(false)
{
}

/**
 * @brief Destructor.
 *
 * The Client does NOT close its own file descriptor.
 * Closing the fd is the responsibility of the networking layer (Ayoub),
 * which calls close(fd) before or after calling Server::removeClient().
 *
 * This keeps the Client class focused on data, not I/O.
 */
Client::~Client()
{
}

/* ************************************************************************** */
/*                               GETTERS                                      */
/* ************************************************************************** */

/**
 * @brief Returns the client's socket file descriptor.
 */
int	Client::getFd() const
{
	return _fd;
}

/**
 * @brief Returns the client's IP address.
 */
const std::string&	Client::getIp() const
{
	return _ip;
}

/**
 * @brief Returns true if the client has completed registration.
 *
 * Registration means PASS + NICK + USER were all accepted successfully.
 */
bool	Client::isRegistered() const
{
	return _registered;
}

/**
 * @brief Returns a reference to the client's buffer manager.
 */
BufferManager&	Client::getBuffer()
{
	return _buffer;
}

/**
 * @brief Returns a const reference to the client's buffer manager.
 */
const BufferManager&	Client::getBuffer() const
{
	return _buffer;
}

/* ---- Phase 5: Registration getters ---- */

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

/**
 * @brief Returns true if the client has set a nickname via NICK.
 */
bool	Client::hasNick() const
{
	return !_nickname.empty();
}

/**
 * @brief Returns true if the client has sent USER (username set).
 */
bool	Client::hasUser() const
{
	return !_username.empty();
}

/* ************************************************************************** */
/*                               SETTERS                                      */
/* ************************************************************************** */

/**
 * @brief Sets the registration state of the client.
 * @param state  true if the client has completed registration.
 *
 * Called by CommandDispatcher once PASS + NICK + USER are all satisfied.
 */
void	Client::setRegistered(bool state)
{
	_registered = state;
}

/* ---- Phase 5: Registration setters ---- */

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
