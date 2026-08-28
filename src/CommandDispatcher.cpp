/* ************************************************************************** */
/*                                                                            */
/*   CommandDispatcher.cpp                                                    */
/*                                                                            */
/*   Phase 5: Command Dispatcher & Registration                               */
/*   Routes parsed IRC commands to their handler functions                    */
/*                                                                            */
/*   Owner: Ismail                                                            */
/*                                                                            */
/* ************************************************************************** */

#include "CommandDispatcher.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Replies.hpp"
#include <iostream>
#include <cctype>
#include <sstream>

/* ************************************************************************** */
/*                          CONSTRUCTORS / DESTRUCTOR                         */
/* ************************************************************************** */

CommandDispatcher::CommandDispatcher(Server& server)
	: _server(server)
{
}

CommandDispatcher::~CommandDispatcher()
{
}

/* ************************************************************************** */
/*                          MAIN DISPATCH                                     */
/* ************************************************************************** */

/**
 * @brief Route a parsed command to the appropriate handler.
 *
 * @param fd   The file descriptor of the sending client.
 * @param cmd  The already-parsed IRC command.
 *
 * Unregistered clients may ONLY send PASS, NICK, USER, QUIT.
 * Every other command is silently ignored (or returns ERR_NOTREGISTERED
 * if we choose to implement that later).
 *
 * Commands are already upper-cased by the Parser.
 */
void CommandDispatcher::dispatch(int fd, const ParsedCommand& cmd)
{
	Client* client = _server.getClient(fd);
	if (!client)
		return;

	/* Allow these commands before registration is complete */
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

	/* --- All other commands require a completed registration --- */
	if (!client->isRegistered())
	{
		/* IRC: ignore pre-registration commands silently, or send 451 */
		/* We send nothing for now to avoid spamming nc testers.        */
		return;
	}

	/* --- Registered-only commands (Phase 6/7) --- */
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

	/* ERR_UNKNOWNCOMMAND for anything we don't handle yet */
	_server.sendToClient(fd,
		Replies::errUnknownCommand(client->getNickname(), cmd.command));
}

/* ************************************************************************** */
/*                        REGISTRATION HANDLERS                               */
/* ************************************************************************** */

/**
 * @brief Handle PASS <password>
 *
 * RFC 2812 §3.1.1 — must be sent BEFORE NICK / USER.
 * If PASS is wrong the client is not disconnected immediately; they can retry
 * (most clients send PASS once, so this is fine).
 * If already registered, send 462 ERR_ALREADYREGISTRED.
 */
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

/**
 * @brief Handle NICK <nickname>
 *
 * RFC 2812 §3.1.2.
 * Rules enforced here:
 *  - Nickname must be non-empty.
 *  - Nickname must pass the validity check (no spaces, reasonable chars).
 *  - Nickname must not already be used by another connected client.
 *
 * After NICK the dispatcher checks whether registration is now complete.
 */
void CommandDispatcher::_handleNick(int fd, Client* client,
                                    const ParsedCommand& cmd)
{
	/* Clients may change NICK after registration too */
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

	/* Announce nick change to the client (and later to channels) */
	if (!client->getNickname().empty())
	{
		/* :<old_nick>!<user>@<host> NICK <new_nick> */
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

/**
 * @brief Handle USER <username> <hostname> <servername> :<realname>
 *
 * RFC 2812 §3.1.3.
 * Needs at least 4 parameters; we use params[0] (username) and
 * params[3] / trailing (realname).
 * If already registered: 462 ERR_ALREADYREGISTRED.
 */
void CommandDispatcher::_handleUser(int fd, Client* client,
                                    const ParsedCommand& cmd)
{
	if (client->isRegistered())
	{
		_server.sendToClient(fd,
			Replies::errAlreadyRegistered(client->getNickname()));
		return;
	}

	/* USER needs: <username> <hostname> <servername> :<realname> */
	if (cmd.params.size() < 4)
	{
		_server.sendToClient(fd,
			Replies::errNeedMoreParams(
				client->getNickname().empty() ? "*" : client->getNickname(),
				"USER"));
		return;
	}

	client->setUsername(cmd.params[0]);
	/* params[3] is the realname (possibly the trailing field) */
	client->setRealname(cmd.params[3]);

	std::cout << "[fd=" << fd << "] USER set: "
	          << cmd.params[0] << " / " << cmd.params[3] << std::endl;

	_tryCompleteRegistration(fd, client);
}

/**
 * @brief Handle QUIT [:<message>]
 *
 * RFC 2812 §3.1.7.
 * Sends a QUIT notice, then disconnects the client.
 */
void CommandDispatcher::_handleQuit(int fd, Client* client,
                                    const ParsedCommand& cmd)
{
	std::string reason = "Client Quit";
	if (!cmd.params.empty())
		reason = cmd.params[0];

	/* :<nick>!<user>@<host> QUIT :<reason> */
	const std::string nick =
		client->getNickname().empty() ? "*" : client->getNickname();
	const std::string user =
		client->getUsername().empty() ? "*" : client->getUsername();

	std::string quitMsg =
		":" + nick + "!" + user + "@" + client->getIp()
		+ " QUIT :" + reason;

	std::cout << "[fd=" << fd << "] QUIT: " << reason << std::endl;

	// Broadcast to all channels the client is in and remove them
	std::vector<std::string> channelsToDelete;
	const std::map<std::string, Channel*>& channels = _server.getChannels();
	for (std::map<std::string, Channel*>::const_iterator it = channels.begin();
	     it != channels.end(); ++it)
	{
		Channel* chan = it->second;
		if (chan->hasMember(fd))
		{
			chan->broadcast(quitMsg, fd);
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

/* ************************************************************************** */
/*                              HELPERS                                       */
/* ************************************************************************** */

/**
 * @brief Validate an IRC nickname per RFC 2812 §2.3.1.
 *
 * <nick> ::= ( letter / special ) *8( letter / digit / special / "-" )
 *
 * letter  = A-Z / a-z
 * digit   = 0-9
 * special = "[" / "]" / "\" / "`" / "_" / "^" / "{" / "|" / "}"
 *
 * Max 9 characters by RFC, but many servers allow up to 30. We enforce 30.
 */
bool CommandDispatcher::_isValidNickname(const std::string& nick) const
{
	if (nick.empty() || nick.size() > 30)
		return false;

	/* First character must be a letter or special */
	char first = nick[0];
	bool firstOk = std::isalpha(static_cast<unsigned char>(first))
	               || first == '[' || first == ']' || first == '\\'
	               || first == '`' || first == '_' || first == '^'
	               || first == '{' || first == '|' || first == '}';
	if (!firstOk)
		return false;

	/* Remaining characters */
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

/**
 * @brief Compare two nicknames case-insensitively using IRC case mapping.
 * RFC 2812 §2.2: {}|^ are lowercase equivalents of []\~
 */
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

/**
 * @brief Check whether a nickname is already in use by another client.
 *
 * @param nick       Nickname to check (case-insensitive per IRC).
 * @param excludeFd  Skip this fd (the client changing their own nick).
 * @return true if some other client already uses this nickname.
 */
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

/**
 * @brief Check if the client has sent everything needed to register,
 *        and if so, complete registration and send 001–004.
 *
 * Registration is complete when:
 *  - PASS was accepted (passReceived == true), AND
 *  - NICK was set (hasNick() == true), AND
 *  - USER was set (hasUser() == true).
 */
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

	/* RFC 2812 §5.1 — send 001, 002, 003, 004 on successful registration */
	_server.sendToClient(fd, Replies::welcome(nick, user, host));
	_server.sendToClient(fd, Replies::yourHost(nick));
	_server.sendToClient(fd, Replies::created(nick, "today"));
	_server.sendToClient(fd, Replies::myInfo(nick));

	std::cout << "[fd=" << fd << "] Client registered as " << nick << std::endl;
}

/* ************************************************************************** */
/*                          PHASE 6 COMMAND HANDLERS                         */
/* ************************************************************************** */

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

		// Validate name starts with # or &
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

		// Check mode checks if it's not new
		if (!isNew)
		{
			// Check limit
			if (chan->isFull())
			{
				_server.sendToClient(fd, Replies::errChannelIsFull(nick, chanName));
				continue;
			}
			// Check invite-only
			if (chan->isInviteOnly() && !chan->isInvited(fd))
			{
				_server.sendToClient(fd, Replies::errInviteOnlyChan(nick, chanName));
				continue;
			}
			// Check key
			if (!chan->getKey().empty() && chan->getKey() != key)
			{
				_server.sendToClient(fd, Replies::errBadChannelKey(nick, chanName));
				continue;
			}
		}

		// Add member
		chan->addMember(client);
		if (chan->isInvited(fd))
			chan->removeInvite(fd);

		if (isNew)
		{
			chan->addOperator(fd);
		}

		// Broadcast join to channel
		std::string joinMsg = ":" + nick + "!" + client->getUsername() + "@" + client->getIp() + " JOIN :" + chanName;
		chan->broadcast(joinMsg, -1);

		// Send topic
		if (chan->getTopic().empty())
			_server.sendToClient(fd, Replies::noTopic(nick, chanName));
		else
			_server.sendToClient(fd, Replies::topic(nick, chanName, chan->getTopic()));

		// Send names list
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

		chan->broadcast(partMsg, -1);
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

	// Target is channel
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
		chan->broadcast(msg, fd);
	}
	else
	{
		// Target is user
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
		chan->broadcast(msg, fd);
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

	// View topic
	if (cmd.params.size() == 1)
	{
		if (chan->getTopic().empty())
			_server.sendToClient(fd, Replies::noTopic(nick, chanName));
		else
			_server.sendToClient(fd, Replies::topic(nick, chanName, chan->getTopic()));
	}
	else
	{
		// Set topic
		if (chan->isTopicRestricted() && !chan->isOperator(fd))
		{
			_server.sendToClient(fd, Replies::errChanOpPrivsNeeded(nick, chanName));
			return;
		}
		std::string newTopic = cmd.params[1];
		chan->setTopic(newTopic);
		std::string topicMsg = ":" + nick + "!" + client->getUsername() + "@" + client->getIp() + " TOPIC " + chanName + " :" + newTopic;
		chan->broadcast(topicMsg, -1);
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
	chan->broadcast(kickMsg, -1);
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
		if (activeModes == "+") activeModes = "";
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
				_server.sendToClient(fd, Replies::errNeedMoreParams(nick, "MODE +o/-o"));
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
		chan->broadcast(modeAnn, -1);
	}
}
