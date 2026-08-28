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

/* ************************************************************************** */
/*                          CONSTRUCTORS / DESTRUCTOR                         */
/* ************************************************************************** */

Channel::Channel(const std::string& name)
	: _name(name), _topic(""), _inviteOnly(false), _topicRestricted(false), _key(""), _limit(-1)
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
/*                          MODE AND INVITE MANAGEMENT                       */
/* ************************************************************************** */

bool Channel::isInviteOnly() const
{
	return _inviteOnly;
}

void Channel::setInviteOnly(bool state)
{
	_inviteOnly = state;
}

bool Channel::isTopicRestricted() const
{
	return _topicRestricted;
}

void Channel::setTopicRestricted(bool state)
{
	_topicRestricted = state;
}

const std::string& Channel::getKey() const
{
	return _key;
}

void Channel::setKey(const std::string& key)
{
	_key = key;
}

int Channel::getLimit() const
{
	return _limit;
}

void Channel::setLimit(int limit)
{
	_limit = limit;
}

void Channel::addInvite(int fd)
{
	if (!isInvited(fd))
		_invitedFds.push_back(fd);
}

void Channel::removeInvite(int fd)
{
	_invitedFds.erase(
		std::remove(_invitedFds.begin(), _invitedFds.end(), fd),
		_invitedFds.end());
}

bool Channel::isInvited(int fd) const
{
	for (size_t i = 0; i < _invitedFds.size(); ++i)
	{
		if (_invitedFds[i] == fd)
			return true;
	}
	return false;
}

bool Channel::isFull() const
{
	if (_limit > 0 && static_cast<int>(_members.size()) >= _limit)
		return true;
	return false;
}
