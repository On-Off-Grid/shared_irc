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
 * Registration starts as false.  It will be set to true by the
 * authentication phase (PASS + NICK + USER) implemented in a later phase.
 */
Client::Client(int fd, const std::string& ip)
	: _fd(fd), _ip(ip), _registered(false)
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
 * In later phases, registration means PASS + NICK + USER were all
 * accepted successfully.  In Phase 3 this always returns false.
 */
bool	Client::isRegistered() const
{
	return _registered;
}

/* ************************************************************************** */
/*                               SETTERS                                      */
/* ************************************************************************** */

/**
 * @brief Sets the registration state of the client.
 * @param state  true if the client has completed registration.
 *
 * Called by the authentication logic in later phases.
 */
void	Client::setRegistered(bool state)
{
	_registered = state;
}
