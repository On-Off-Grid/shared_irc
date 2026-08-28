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
#include "Replies.hpp"
#include <iostream>
#include <cctype>

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

	/* --- Registered-only commands (Phase 7+) --- */
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

	_server.sendToClient(fd, quitMsg);

	std::cout << "[fd=" << fd << "] QUIT: " << reason << std::endl;

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
