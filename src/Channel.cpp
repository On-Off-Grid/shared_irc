#include "Channel.hpp"
#include "Client.hpp"
#include <algorithm>

Channel::Channel(const std::string& name)
	: _name(name), _topic(""), _inviteOnly(false), _topicRestricted(false), _key(""), _limit(-1)
{
}

Channel::~Channel()
{
	_members.clear();
	_operators.clear();
}

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


void Channel::addMember(Client* client)
{
	if (!client)
		return;
	_members[client->getFd()] = client;
}


void Channel::removeMember(int fd)
{
	_members.erase(fd);
	removeOperator(fd);
}

bool Channel::hasMember(int fd) const
{
	return _members.find(fd) != _members.end();
}

Client* Channel::getMember(int fd) const
{
	std::map<int, Client*>::const_iterator it = _members.find(fd);
	if (it == _members.end())
		return NULL;
	return it->second;
}

void Channel::addOperator(int fd)
{
	if (!isOperator(fd))
		_operators.push_back(fd);
}

void Channel::removeOperator(int fd)
{
	_operators.erase(
		std::remove(_operators.begin(), _operators.end(), fd),
		_operators.end());
}

bool Channel::isOperator(int fd) const
{
	for (size_t i = 0; i < _operators.size(); ++i)
	{
		if (_operators[i] == fd)
			return true;
	}
	return false;
}

void Channel::setTopic(const std::string& topic)
{
	_topic = topic;
}

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
