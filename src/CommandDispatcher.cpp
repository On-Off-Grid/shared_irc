#include "CommandDispatcher.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Replies.hpp"
#include <iostream>
#include <cctype>
#include <sstream>

CommandDispatcher::CommandDispatcher(Server& server)
	: _server(server)
{
}

CommandDispatcher::~CommandDispatcher()
{
}

void CommandDispatcher::dispatch(int fd, const ParsedCommand& cmd)
{
	Client* client = _server.getClient(fd);
	if (!client)
		return;
	if (cmd.command == "PASS")
	{
		_handlePass(fd, client, cmd);
		return;
	}
	if (cmd.command == "NICK")
	{
		_handleNick(fd, client, cmd);
		return;
	}
	if (cmd.command == "USER")
	{
		_handleUser(fd, client, cmd);
		return;
	}
	if (cmd.command == "QUIT")
	{
		_handleQuit(fd, client, cmd);
		return;
	}

	if (!client->isRegistered())
	{
		return;
	}

	if (cmd.command == "JOIN")
	{
		_handleJoin(fd, client, cmd);
		return;
	}
	if (cmd.command == "PART")
	{
		_handlePart(fd, client, cmd);
		return;
	}
	if (cmd.command == "PRIVMSG")
	{
		_handlePrivmsg(fd, client, cmd);
		return;
	}
	if (cmd.command == "NOTICE")
	{
		_handleNotice(fd, client, cmd);
		return;
	}
	if (cmd.command == "TOPIC")
	{
		_handleTopic(fd, client, cmd);
		return;
	}
	if (cmd.command == "KICK")
	{
		_handleKick(fd, client, cmd);
		return;
	}
	if (cmd.command == "INVITE")
	{
		_handleInvite(fd, client, cmd);
		return;
	}
	if (cmd.command == "MODE")
	{
		_handleMode(fd, client, cmd);
		return;
	}

	_server.sendToClient(fd,
		Replies::errUnknownCommand(client->getNickname(), cmd.command));
}

void CommandDispatcher::_handlePass(int fd, Client* client,
                                    const ParsedCommand& cmd)
{
	if (client->isRegistered())
	{
		_server.sendToClient(fd,
			Replies::errAlreadyRegistered(client->getNickname()));
		return;
	}

	if (cmd.params.empty())
	{
		_server.sendToClient(fd,
			Replies::errNeedMoreParams(
				client->getNickname().empty() ? "*" : client->getNickname(),
				"PASS"));
		return;
	}

	if (cmd.params[0] == _server.getPassword())
	{
		client->setPassReceived(true);
		std::cout << "[fd=" << fd << "] PASS accepted." << std::endl;
		_tryCompleteRegistration(fd, client);
	}
	else
	{
		client->setPassReceived(false);
		_server.sendToClient(fd,
			Replies::errPasswdMismatch(
				client->getNickname().empty() ? "*" : client->getNickname()));
		std::cout << "[fd=" << fd << "] PASS rejected." << std::endl;
	}
}

void CommandDispatcher::_handleNick(int fd, Client* client,
                                    const ParsedCommand& cmd)
{
	const std::string currentNick =
		client->getNickname().empty() ? "*" : client->getNickname();

	if (cmd.params.empty() || cmd.params[0].empty())
	{
		_server.sendToClient(fd,
			Replies::errNoNicknameGiven(currentNick));
		return;
	}

	const std::string& newNick = cmd.params[0];

	if (!_isValidNickname(newNick))
	{
		_server.sendToClient(fd,
			Replies::errErroneusNickname(currentNick, newNick));
		return;
	}

	if (_isNicknameInUse(newNick, fd))
	{
		_server.sendToClient(fd,
			Replies::errNicknameInUse(currentNick, newNick));
		return;
	}
	if (!client->getNickname().empty())
	{
		std::string announcement =
			":" + client->getNickname()
			+ "!" + (client->getUsername().empty() ? "*" : client->getUsername())
			+ "@" + client->getIp()
			+ " NICK :" + newNick;
		_server.sendToClient(fd, announcement);
	}

	client->setNickname(newNick);
	std::cout << "[fd=" << fd << "] NICK set to: " << newNick << std::endl;

	_tryCompleteRegistration(fd, client);
}

void CommandDispatcher::_handleUser(int fd, Client* client,
                                    const ParsedCommand& cmd)
{
	if (client->isRegistered())
	{
		_server.sendToClient(fd,
			Replies::errAlreadyRegistered(client->getNickname()));
		return;
	}

	if (cmd.params.size() < 4)
	{
		_server.sendToClient(fd,
			Replies::errNeedMoreParams(
				client->getNickname().empty() ? "*" : client->getNickname(),
				"USER"));
		return;
	}

	client->setUsername(cmd.params[0]);
	client->setRealname(cmd.params[3]);

	std::cout << "[fd=" << fd << "] USER set: "
	          << cmd.params[0] << " / " << cmd.params[3] << std::endl;

	_tryCompleteRegistration(fd, client);
}

void CommandDispatcher::_handleQuit(int fd, Client* client,
                                    const ParsedCommand& cmd)
{
	std::string reason = "Client Quit";
	if (!cmd.params.empty())
		reason = cmd.params[0];
	const std::string nick =
		client->getNickname().empty() ? "*" : client->getNickname();
	const std::string user =
		client->getUsername().empty() ? "*" : client->getUsername();

	std::string quitMsg =
		":" + nick + "!" + user + "@" + client->getIp()
		+ " QUIT :" + reason;

	std::cout << "[fd=" << fd << "] QUIT: " << reason << std::endl;

	std::vector<std::string> channelsToDelete;
	const std::map<std::string, Channel*>& channels = _server.getChannels();
	for (std::map<std::string, Channel*>::const_iterator it = channels.begin();
	     it != channels.end(); ++it)
	{
		Channel* chan = it->second;
		if (chan->hasMember(fd))
		{
			_server.broadcastToChannel(*chan, quitMsg, fd);
			chan->removeMember(fd);
		}
		if (chan->getMembers().empty())
		{
			channelsToDelete.push_back(chan->getName());
		}
	}
	for (size_t i = 0; i < channelsToDelete.size(); ++i)
	{
		_server.deleteChannel(channelsToDelete[i]);
	}

	_server.sendToClient(fd, quitMsg);

	/* Let Server handle the actual socket/poll cleanup */
	_server.disconnectClient(fd);
}

bool CommandDispatcher::_isValidNickname(const std::string& nick) const
{
	if (nick.empty() || nick.size() > 30)
		return false;

	char first = nick[0];
	bool firstOk = std::isalpha(static_cast<unsigned char>(first))
	               || first == '[' || first == ']' || first == '\\'
	               || first == '`' || first == '_' || first == '^'
	               || first == '{' || first == '|' || first == '}';
	if (!firstOk)
		return false;

	for (size_t i = 1; i < nick.size(); ++i)
	{
		char c = nick[i];
		bool ok = std::isalnum(static_cast<unsigned char>(c))
		          || c == '[' || c == ']' || c == '\\'
		          || c == '`' || c == '_' || c == '^'
		          || c == '{' || c == '|' || c == '}'
		          || c == '-';
		if (!ok)
			return false;
	}
	return true;
}

static bool areNicknamesEqual(const std::string& n1, const std::string& n2)
{
	if (n1.size() != n2.size())
		return false;
	for (size_t i = 0; i < n1.size(); ++i)
	{
		char c1 = n1[i];
		char c2 = n2[i];

		if (c1 >= 'A' && c1 <= 'Z') c1 = c1 - 'A' + 'a';
		if (c2 >= 'A' && c2 <= 'Z') c2 = c2 - 'A' + 'a';

		if (c1 == '[') c1 = '{';
		else if (c1 == ']') c1 = '}';
		else if (c1 == '\\') c1 = '|';
		else if (c1 == '^') c1 = '~';

		if (c2 == '[') c2 = '{';
		else if (c2 == ']') c2 = '}';
		else if (c2 == '\\') c2 = '|';
		else if (c2 == '^') c2 = '~';

		if (c1 != c2)
			return false;
	}
	return true;
}

bool CommandDispatcher::_isNicknameInUse(const std::string& nick,
                                          int excludeFd) const
{
	const std::map<int, Client*>& clients = _server.getClients();
	for (std::map<int, Client*>::const_iterator it = clients.begin();
	     it != clients.end(); ++it)
	{
		if (it->first == excludeFd)
			continue;
		if (areNicknamesEqual(it->second->getNickname(), nick))
			return true;
	}
	return false;
}

void CommandDispatcher::_tryCompleteRegistration(int fd, Client* client)
{
	if (client->isRegistered())
		return;
	if (!client->hasReceivedPass() || !client->hasNick() || !client->hasUser())
		return;

	client->setRegistered(true);

	const std::string& nick = client->getNickname();
	const std::string& user = client->getUsername();
	const std::string& host = client->getIp();

	_server.sendToClient(fd, Replies::welcome(nick, user, host));
	_server.sendToClient(fd, Replies::yourHost(nick));
	_server.sendToClient(fd, Replies::created(nick, "today"));
	_server.sendToClient(fd, Replies::myInfo(nick));

	std::cout << "[fd=" << fd << "] Client registered as " << nick << std::endl;
}

static std::vector<std::string> _split(const std::string& str, char delim)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(str);
	while (std::getline(tokenStream, token, delim))
	{
		tokens.push_back(token);
	}
	return tokens;
}

void CommandDispatcher::_handleJoin(int fd, Client* client, const ParsedCommand& cmd)
{
	std::string nick = client->getNickname();
	if (cmd.params.empty())
	{
		_server.sendToClient(fd, Replies::errNeedMoreParams(nick, "JOIN"));
		return;
	}

	std::vector<std::string> channels = _split(cmd.params[0], ',');
	std::vector<std::string> keys;
	if (cmd.params.size() > 1)
		keys = _split(cmd.params[1], ',');

	for (size_t i = 0; i < channels.size(); ++i)
	{
		std::string chanName = channels[i];
		if (chanName.empty())
			continue;

		if (chanName[0] != '#' && chanName[0] != '&')
		{
			_server.sendToClient(fd, Replies::errNoSuchChannel(nick, chanName));
			continue;
		}

		std::string key = "";
		if (i < keys.size())
			key = keys[i];

		Channel* chan = _server.getChannel(chanName);
		bool isNew = (chan == NULL);

		if (isNew)
		{
			chan = _server.createChannel(chanName);
		}

		if (!isNew)
		{
			if (chan->isFull())
			{
				_server.sendToClient(fd, Replies::errChannelIsFull(nick, chanName));
				continue;
			}
			if (chan->isInviteOnly() && !chan->isInvited(fd))
			{
				_server.sendToClient(fd, Replies::errInviteOnlyChan(nick, chanName));
				continue;
			}
			if (!chan->getKey().empty() && chan->getKey() != key)
			{
				_server.sendToClient(fd, Replies::errBadChannelKey(nick, chanName));
				continue;
			}
		}

		chan->addMember(client);
		if (chan->isInvited(fd))
			chan->removeInvite(fd);

		if (isNew)
		{
			chan->addOperator(fd);
		}

		std::string joinMsg = ":" + nick + "!" + client->getUsername() + "@" + client->getIp() + " JOIN :" + chanName;
		_server.broadcastToChannel(*chan, joinMsg, -1);

		if (chan->getTopic().empty())
			_server.sendToClient(fd, Replies::noTopic(nick, chanName));
		else
			_server.sendToClient(fd, Replies::topic(nick, chanName, chan->getTopic()));

		std::string namesList = "";
		const std::map<int, Client*>& members = chan->getMembers();
		for (std::map<int, Client*>::const_iterator mit = members.begin(); mit != members.end(); ++mit)
		{
			if (!namesList.empty())
				namesList += " ";
			if (chan->isOperator(mit->first))
				namesList += "@";
			namesList += mit->second->getNickname();
		}
		_server.sendToClient(fd, Replies::namReply(nick, chanName, namesList));
		_server.sendToClient(fd, Replies::endOfNames(nick, chanName));
	}
}

void CommandDispatcher::_handlePart(int fd, Client* client, const ParsedCommand& cmd)
{
	std::string nick = client->getNickname();
	if (cmd.params.empty())
	{
		_server.sendToClient(fd, Replies::errNeedMoreParams(nick, "PART"));
		return;
	}

	std::string reason = "";
	if (cmd.params.size() > 1)
		reason = cmd.params[1];

	std::vector<std::string> channels = _split(cmd.params[0], ',');
	for (size_t i = 0; i < channels.size(); ++i)
	{
		std::string chanName = channels[i];
		if (chanName.empty())
			continue;

		Channel* chan = _server.getChannel(chanName);
		if (!chan)
		{
			_server.sendToClient(fd, Replies::errNoSuchChannel(nick, chanName));
			continue;
		}

		if (!chan->hasMember(fd))
		{
			_server.sendToClient(fd, Replies::errNotOnChannel(nick, chanName));
			continue;
		}

		std::string partMsg = ":" + nick + "!" + client->getUsername() + "@" + client->getIp() + " PART " + chanName;
		if (!reason.empty())
			partMsg += " :" + reason;

		_server.broadcastToChannel(*chan, partMsg, -1);
		chan->removeMember(fd);

		if (chan->getMembers().empty())
		{
			_server.deleteChannel(chanName);
		}
	}
}

void CommandDispatcher::_handlePrivmsg(int fd, Client* client, const ParsedCommand& cmd)
{
	std::string nick = client->getNickname();
	if (cmd.params.size() < 2 || cmd.params[0].empty() || cmd.params[1].empty())
	{
		_server.sendToClient(fd, Replies::errNeedMoreParams(nick, "PRIVMSG"));
		return;
	}

	std::string target = cmd.params[0];
	std::string message = cmd.params[1];

	if (target[0] == '#' || target[0] == '&')
	{
		Channel* chan = _server.getChannel(target);
		if (!chan)
		{
			_server.sendToClient(fd, Replies::errNoSuchChannel(nick, target));
			return;
		}
		if (!chan->hasMember(fd))
		{
			_server.sendToClient(fd, Replies::errCannotSendToChan(nick, target));
			return;
		}
		std::string msg = ":" + nick + "!" + client->getUsername() + "@" + client->getIp() + " PRIVMSG " + target + " :" + message;
		_server.broadcastToChannel(*chan, msg, fd);
	}
	else
	{
		Client* targetClient = _server.getClientByNickname(target);
		if (!targetClient)
		{
			_server.sendToClient(fd, Replies::errNoSuchNick(nick, target));
			return;
		}
		std::string msg = ":" + nick + "!" + client->getUsername() + "@" + client->getIp() + " PRIVMSG " + targetClient->getNickname() + " :" + message;
		_server.sendToClient(targetClient->getFd(), msg);
	}
}

void CommandDispatcher::_handleNotice(int fd, Client* client, const ParsedCommand& cmd)
{
	if (cmd.params.size() < 2 || cmd.params[0].empty() || cmd.params[1].empty())
		return;

	std::string nick = client->getNickname();
	std::string target = cmd.params[0];
	std::string message = cmd.params[1];

	if (target[0] == '#' || target[0] == '&')
	{
		Channel* chan = _server.getChannel(target);
		if (!chan || !chan->hasMember(fd))
			return;
		std::string msg = ":" + nick + "!" + client->getUsername() + "@" + client->getIp() + " NOTICE " + target + " :" + message;
		_server.broadcastToChannel(*chan, msg, fd);
	}
	else
	{
		Client* targetClient = _server.getClientByNickname(target);
		if (!targetClient)
			return;
		std::string msg = ":" + nick + "!" + client->getUsername() + "@" + client->getIp() + " NOTICE " + targetClient->getNickname() + " :" + message;
		_server.sendToClient(targetClient->getFd(), msg);
	}
}

void CommandDispatcher::_handleTopic(int fd, Client* client, const ParsedCommand& cmd)
{
	std::string nick = client->getNickname();
	if (cmd.params.empty())
	{
		_server.sendToClient(fd, Replies::errNeedMoreParams(nick, "TOPIC"));
		return;
	}

	std::string chanName = cmd.params[0];
	Channel* chan = _server.getChannel(chanName);
	if (!chan)
	{
		_server.sendToClient(fd, Replies::errNoSuchChannel(nick, chanName));
		return;
	}

	if (!chan->hasMember(fd))
	{
		_server.sendToClient(fd, Replies::errNotOnChannel(nick, chanName));
		return;
	}

	if (cmd.params.size() == 1)
	{
		if (chan->getTopic().empty())
			_server.sendToClient(fd, Replies::noTopic(nick, chanName));
		else
			_server.sendToClient(fd, Replies::topic(nick, chanName, chan->getTopic()));
	}
	else
	{
		if (chan->isTopicRestricted() && !chan->isOperator(fd))
		{
			_server.sendToClient(fd, Replies::errChanOpPrivsNeeded(nick, chanName));
			return;
		}
		std::string newTopic = cmd.params[1];
		chan->setTopic(newTopic);
		std::string topicMsg = ":" + nick + "!" + client->getUsername() + "@" + client->getIp() + " TOPIC " + chanName + " :" + newTopic;
		_server.broadcastToChannel(*chan, topicMsg, -1);
	}
}

void CommandDispatcher::_handleKick(int fd, Client* client, const ParsedCommand& cmd)
{
	std::string nick = client->getNickname();
	if (cmd.params.size() < 2)
	{
		_server.sendToClient(fd, Replies::errNeedMoreParams(nick, "KICK"));
		return;
	}

	std::string chanName = cmd.params[0];
	std::string targetNick = cmd.params[1];
	std::string reason = "Kicked by operator";
	if (cmd.params.size() > 2)
		reason = cmd.params[2];

	Channel* chan = _server.getChannel(chanName);
	if (!chan)
	{
		_server.sendToClient(fd, Replies::errNoSuchChannel(nick, chanName));
		return;
	}

	if (!chan->hasMember(fd))
	{
		_server.sendToClient(fd, Replies::errNotOnChannel(nick, chanName));
		return;
	}

	if (!chan->isOperator(fd))
	{
		_server.sendToClient(fd, Replies::errChanOpPrivsNeeded(nick, chanName));
		return;
	}

	Client* targetClient = _server.getClientByNickname(targetNick);
	if (!targetClient || !chan->hasMember(targetClient->getFd()))
	{
		_server.sendToClient(fd, Replies::errUserNotInChannel(nick, targetNick, chanName));
		return;
	}

	int targetFd = targetClient->getFd();
	std::string kickMsg = ":" + nick + "!" + client->getUsername() + "@" + client->getIp() + " KICK " + chanName + " " + targetNick + " :" + reason;
	_server.broadcastToChannel(*chan, kickMsg, -1);
	chan->removeMember(targetFd);

	if (chan->getMembers().empty())
	{
		_server.deleteChannel(chanName);
	}
}

void CommandDispatcher::_handleInvite(int fd, Client* client, const ParsedCommand& cmd)
{
	std::string nick = client->getNickname();
	if (cmd.params.size() < 2)
	{
		_server.sendToClient(fd, Replies::errNeedMoreParams(nick, "INVITE"));
		return;
	}

	std::string targetNick = cmd.params[0];
	std::string chanName = cmd.params[1];

	Channel* chan = _server.getChannel(chanName);
	if (!chan)
	{
		_server.sendToClient(fd, Replies::errNoSuchChannel(nick, chanName));
		return;
	}

	if (!chan->hasMember(fd))
	{
		_server.sendToClient(fd, Replies::errNotOnChannel(nick, chanName));
		return;
	}

	if (chan->isInviteOnly() && !chan->isOperator(fd))
	{
		_server.sendToClient(fd, Replies::errChanOpPrivsNeeded(nick, chanName));
		return;
	}

	Client* targetClient = _server.getClientByNickname(targetNick);
	if (!targetClient)
	{
		_server.sendToClient(fd, Replies::errNoSuchNick(nick, targetNick));
		return;
	}

	if (chan->hasMember(targetClient->getFd()))
	{
		_server.sendToClient(fd, Replies::errUserOnChannel(nick, targetNick, chanName));
		return;
	}

	chan->addInvite(targetClient->getFd());
	_server.sendToClient(fd, Replies::inviting(nick, targetNick, chanName));

	std::string inviteMsg = ":" + nick + "!" + client->getUsername() + "@" + client->getIp() + " INVITE " + targetNick + " :" + chanName;
	_server.sendToClient(targetClient->getFd(), inviteMsg);
}

void CommandDispatcher::_handleMode(int fd, Client* client, const ParsedCommand& cmd)
{
	std::string nick = client->getNickname();
	if (cmd.params.empty())
	{
		_server.sendToClient(fd, Replies::errNeedMoreParams(nick, "MODE"));
		return;
	}

	std::string target = cmd.params[0];
	if (target[0] != '#' && target[0] != '&')
	{
		return;
	}

	Channel* chan = _server.getChannel(target);
	if (!chan)
	{
		_server.sendToClient(fd, Replies::errNoSuchChannel(nick, target));
		return;
	}

	if (cmd.params.size() == 1)
	{
		std::string activeModes = "+";
		std::string params = "";
		if (chan->isInviteOnly()) activeModes += "i";
		if (chan->isTopicRestricted()) activeModes += "t";
		if (!chan->getKey().empty())
		{
			activeModes += "k";
			if (!params.empty()) params += " ";
			params += chan->getKey();
		}
		if (chan->getLimit() > 0)
		{
			activeModes += "l";
			if (!params.empty()) params += " ";
			std::ostringstream ss;
			ss << chan->getLimit();
			params += ss.str();
		}
		_server.sendToClient(fd, Replies::channelModeIs(nick, target, activeModes, params));
		return;
	}

	if (!chan->isOperator(fd))
	{
		_server.sendToClient(fd, Replies::errChanOpPrivsNeeded(nick, target));
		return;
	}

	std::string modeString = cmd.params[1];
	size_t paramIdx = 2;

	bool sign = true;
	std::string appliedModes = "";
	std::string appliedParams = "";

	for (size_t i = 0; i < modeString.size(); ++i)
	{
		char c = modeString[i];
		if (c != '+' && c != '-' && appliedModes.empty())
			appliedModes += (sign ? "+" : "-");
		if (c == '+')
		{
			sign = true;
			if (appliedModes.empty() || appliedModes[appliedModes.size() - 1] != '+')
				appliedModes += "+";
			continue;
		}
		if (c == '-')
		{
			sign = false;
			if (appliedModes.empty() || appliedModes[appliedModes.size() - 1] != '-')
				appliedModes += "-";
			continue;
		}

		if (c == 'i')
		{
			chan->setInviteOnly(sign);
			appliedModes += "i";
		}
		else if (c == 't')
		{
			chan->setTopicRestricted(sign);
			appliedModes += "t";
		}
		else if (c == 'k')
		{
			if (sign)
			{
				if (paramIdx < cmd.params.size())
				{
					std::string key = cmd.params[paramIdx++];
					chan->setKey(key);
					appliedModes += "k";
					if (!appliedParams.empty()) appliedParams += " ";
					appliedParams += key;
				}
			}
			else
			{
				chan->setKey("");
				appliedModes += "k";
			}
		}
		else if (c == 'l')
		{
			if (sign)
			{
				if (paramIdx < cmd.params.size())
				{
					std::string limitStr = cmd.params[paramIdx++];
					std::istringstream iss(limitStr);
					int limit;
					if (iss >> limit && limit > 0)
					{
						chan->setLimit(limit);
						appliedModes += "l";
						if (!appliedParams.empty()) appliedParams += " ";
						appliedParams += limitStr;
					}
				}
			}
			else
			{
				chan->setLimit(-1);
				appliedModes += "l";
			}
		}
		else if (c == 'o')
		{
			if (paramIdx < cmd.params.size())
			{
				std::string opNick = cmd.params[paramIdx++];
				Client* opClient = _server.getClientByNickname(opNick);
				if (opClient && chan->hasMember(opClient->getFd()))
				{
					if (sign)
						chan->addOperator(opClient->getFd());
					else
						chan->removeOperator(opClient->getFd());
					appliedModes += "o";
					if (!appliedParams.empty()) appliedParams += " ";
					appliedParams += opNick;
				}
				else
				{
					_server.sendToClient(fd, Replies::errUserNotInChannel(nick, opNick, target));
				}
			}
			else
			{
				_server.sendToClient(fd, Replies::errNeedMoreParams(nick, "MODE"));
			}
		}
	}

	if (!appliedModes.empty() && (appliedModes[appliedModes.size() - 1] == '+' || appliedModes[appliedModes.size() - 1] == '-'))
	{
		appliedModes.erase(appliedModes.size() - 1);
	}

	if (!appliedModes.empty())
	{
		std::string modeAnn = ":" + nick + "!" + client->getUsername() + "@" + client->getIp() + " MODE " + target + " " + appliedModes;
		if (!appliedParams.empty())
			modeAnn += " " + appliedParams;
		_server.broadcastToChannel(*chan, modeAnn, -1);
	}
}
