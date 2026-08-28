/* ************************************************************************** */
/*                                                                            */
/*   Channel.cpp                                                              */
/*                                                                            */
/*   Phase 5: Command Dispatcher & Registration                               */
/*   Channel class implementation                                             */
/*                                                                            */
/*   Owner: Ismail                                                            */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include "Client.hpp"
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <iostream>

/* ************************************************************************** */
/*                          CONSTRUCTORS / DESTRUCTOR                         */
/* ************************************************************************** */

Channel::Channel(const std::string& name)
	: _name(name), _topic("")
{
}

Channel::~Channel()
{
	/* Members are NOT owned by Channel — Client* are owned by Server */
	_members.clear();
	_operators.clear();
}

/* ************************************************************************** */
/*                               GETTERS                                      */
/* ************************************************************************** */

const std::string& Channel::getName() const
{
	return _name;
}

const std::string& Channel::getTopic() const
{
	return _topic;
}

const std::map<int, Client*>& Channel::getMembers() const
{
	return _members;
}

/* ************************************************************************** */
/*                          MEMBER MANAGEMENT                                 */
/* ************************************************************************** */

/**
 * @brief Add a client to this channel.
 * @param client  Pointer to the Client (owned by Server, not by Channel).
 */
void Channel::addMember(Client* client)
{
	if (!client)
		return;
	_members[client->getFd()] = client;
}

/**
 * @brief Remove a client from this channel by fd.
 * @param fd  The file descriptor of the client to remove.
 */
void Channel::removeMember(int fd)
{
	_members.erase(fd);
	removeOperator(fd);
}

/**
 * @brief Check whether a client is in this channel.
 * @param fd  File descriptor to check.
 * @return true if the client is a member.
 */
bool Channel::hasMember(int fd) const
{
	return _members.find(fd) != _members.end();
}

/**
 * @brief Look up a channel member by fd.
 * @param fd  File descriptor.
 * @return Pointer to Client, or NULL if not found.
 */
Client* Channel::getMember(int fd) const
{
	std::map<int, Client*>::const_iterator it = _members.find(fd);
	if (it == _members.end())
		return NULL;
	return it->second;
}

/* ************************************************************************** */
/*                          OPERATOR MANAGEMENT                               */
/* ************************************************************************** */

/**
 * @brief Give channel operator status to a member.
 * @param fd  The file descriptor of the client to promote.
 */
void Channel::addOperator(int fd)
{
	if (!isOperator(fd))
		_operators.push_back(fd);
}

/**
 * @brief Revoke channel operator status from a member.
 * @param fd  The file descriptor of the client to demote.
 */
void Channel::removeOperator(int fd)
{
	_operators.erase(
		std::remove(_operators.begin(), _operators.end(), fd),
		_operators.end());
}

/**
 * @brief Check whether a client is a channel operator.
 * @param fd  File descriptor to check.
 * @return true if the client is an operator.
 */
bool Channel::isOperator(int fd) const
{
	for (size_t i = 0; i < _operators.size(); ++i)
	{
		if (_operators[i] == fd)
			return true;
	}
	return false;
}

/* ************************************************************************** */
/*                               TOPIC                                        */
/* ************************************************************************** */

void Channel::setTopic(const std::string& topic)
{
	_topic = topic;
}

/* ************************************************************************** */
/*                              BROADCAST                                     */
/* ************************************************************************** */

/**
 * @brief Send a message to every member of the channel.
 *
 * @param message    The raw IRC message string (without CRLF).
 * @param excludeFd  fd to skip (typically the sender). Pass -1 to send to all.
 *
 * Each member's send is handled by the Client fd — the Server's
 * sendToClient() method appends CRLF and calls send().
 * Here we write directly to the fd so Channel stays decoupled from Server.
 *
 * NOTE: Phase 7 will wire this through Server::sendToClient() for proper
 *       send-queue handling. For Phase 5 we write directly with send().
 */
#include <sys/socket.h>
#include <string>

void Channel::broadcast(const std::string& message, int excludeFd) const
{
	std::string fullMsg = message + "\r\n";
	for (std::map<int, Client*>::const_iterator it = _members.begin();
	     it != _members.end(); ++it)
	{
		if (it->first == excludeFd)
			continue;

		int fd = it->first;
		size_t totalSent = 0;
		size_t remaining = fullMsg.size();

		while (totalSent < fullMsg.size())
		{
			ssize_t bytesSent = send(fd, fullMsg.c_str() + totalSent, remaining, 0);
			if (bytesSent == 0)
				break;
			if (bytesSent == -1)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
				{
					std::cerr << "send() would block on fd " << fd
							  << " (broadcast) — dropping remaining bytes." << std::endl;
					break;
				}
				if (errno == EINTR)
					continue;
				std::cerr << "send() error on fd " << fd << " (broadcast): "
						  << strerror(errno) << std::endl;
				break;
			}
			totalSent += bytesSent;
			remaining -= bytesSent;
		}
	}
}
