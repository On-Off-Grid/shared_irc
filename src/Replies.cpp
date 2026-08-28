/* ************************************************************************** */
/*                                                                            */
/*   Replies.cpp                                                              */
/*                                                                            */
/*   Phase 5: Command Dispatcher & Registration                               */
/*   IRC numeric reply builders (RFC 1459 / RFC 2812)                        */
/*                                                                            */
/*   Owner: Ismail                                                            */
/*                                                                            */
/* ************************************************************************** */

#include "Replies.hpp"

/**
 * Helper: build the prefix common to every reply.
 *   :<SERVER_NAME> <code> <nick>
 */
static std::string _prefix(const std::string& code, const std::string& nick)
{
	return std::string(":") + SERVER_NAME + " " + code + " " + nick;
}

/* ---- Success replies ---- */

/**
 * 001 RPL_WELCOME
 * :<server> 001 <nick> :Welcome to the IRC Network, <nick>!<user>@<host>
 */
std::string Replies::welcome(const std::string& nick,
                             const std::string& user,
                             const std::string& host)
{
	return _prefix(RPL_WELCOME, nick)
	       + " :Welcome to the IRC Network, "
	       + nick + "!" + user + "@" + host;
}

/**
 * 002 RPL_YOURHOST
 * :<server> 002 <nick> :Your host is <server>, running version ft_irc-1.0
 */
std::string Replies::yourHost(const std::string& nick)
{
	return _prefix(RPL_YOURHOST, nick)
	       + " :Your host is " SERVER_NAME
	         ", running version ft_irc-1.0";
}

/**
 * 003 RPL_CREATED
 * :<server> 003 <nick> :This server was created <date>
 */
std::string Replies::created(const std::string& nick, const std::string& date)
{
	return _prefix(RPL_CREATED, nick)
	       + " :This server was created " + date;
}

/**
 * 004 RPL_MYINFO
 * :<server> 004 <nick> <server> ft_irc-1.0 o o
 */
std::string Replies::myInfo(const std::string& nick)
{
	return _prefix(RPL_MYINFO, nick)
	       + " " SERVER_NAME " ft_irc-1.0 o o";
}

/* ---- Error replies ---- */

/**
 * 461 ERR_NEEDMOREPARAMS
 * :<server> 461 <nick> <cmd> :Not enough parameters
 */
std::string Replies::errNeedMoreParams(const std::string& nick,
                                       const std::string& cmd)
{
	return _prefix(ERR_NEEDMOREPARAMS, nick)
	       + " " + cmd + " :Not enough parameters";
}

/**
 * 462 ERR_ALREADYREGISTRED
 * :<server> 462 <nick> :Unauthorized command (already registered)
 */
std::string Replies::errAlreadyRegistered(const std::string& nick)
{
	return _prefix(ERR_ALREADYREGISTRED, nick)
	       + " :Unauthorized command (already registered)";
}

/**
 * 464 ERR_PASSWDMISMATCH
 * :<server> 464 <nick> :Password incorrect
 */
std::string Replies::errPasswdMismatch(const std::string& nick)
{
	return _prefix(ERR_PASSWDMISMATCH, nick)
	       + " :Password incorrect";
}

/**
 * 431 ERR_NONICKNAMEGIVEN
 * :<server> 431 <nick> :No nickname given
 */
std::string Replies::errNoNicknameGiven(const std::string& nick)
{
	return _prefix(ERR_NONICKNAMEGIVEN, nick)
	       + " :No nickname given";
}

/**
 * 432 ERR_ERRONEUSNICKNAME
 * :<server> 432 <nick> <badNick> :Erroneous nickname
 */
std::string Replies::errErroneusNickname(const std::string& nick,
                                          const std::string& badNick)
{
	return _prefix(ERR_ERRONEUSNICK, nick)
	       + " " + badNick + " :Erroneous nickname";
}

/**
 * 433 ERR_NICKNAMEINUSE
 * :<server> 433 <nick> <usedNick> :Nickname is already in use
 */
std::string Replies::errNicknameInUse(const std::string& nick,
                                       const std::string& usedNick)
{
	return _prefix(ERR_NICKNAMEINUSE, nick)
	       + " " + usedNick + " :Nickname is already in use";
}

/**
 * 421 ERR_UNKNOWNCOMMAND
 * :<server> 421 <nick> <cmd> :Unknown command
 */
std::string Replies::errUnknownCommand(const std::string& nick,
                                        const std::string& cmd)
{
	return _prefix(ERR_UNKNOWNCOMMAND, nick)
	       + " " + cmd + " :Unknown command";
}
