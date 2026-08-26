/* ************************************************************************** */
/*                                                                            */
/*   Replies.hpp                                                              */
/*                                                                            */
/*   Phase 5: Command Dispatcher & Registration                               */
/*   IRC numeric reply builders (RFC 1459 / RFC 2812)                        */
/*                                                                            */
/*   Owner: Ismail                                                            */
/*                                                                            */
/* ************************************************************************** */

#ifndef REPLIES_HPP
# define REPLIES_HPP

# include <string>

/* ---- Numeric reply codes ---- */
# define RPL_WELCOME          "001"
# define RPL_YOURHOST         "002"
# define RPL_CREATED          "003"
# define RPL_MYINFO           "004"
# define RPL_TOPIC            "332"
# define RPL_NAMREPLY         "353"
# define RPL_ENDOFNAMES       "366"

# define ERR_NOSUCHNICK       "401"
# define ERR_NOSUCHCHANNEL    "403"
# define ERR_CANNOTSENDTOCHAN "404"
# define ERR_UNKNOWNCOMMAND   "421"
# define ERR_NONICKNAMEGIVEN  "431"
# define ERR_ERRONEUSNICK     "432"
# define ERR_NICKNAMEINUSE    "433"
# define ERR_NOTONCHANNEL     "442"
# define ERR_NEEDMOREPARAMS   "461"
# define ERR_ALREADYREGISTRED "462"
# define ERR_PASSWDMISMATCH   "464"

/* ---- Server name used in replies ---- */
# define SERVER_NAME          "ircserv"

/**
 * @namespace Replies
 * @brief Factory functions that build IRC reply strings.
 *
 * Every function returns a complete IRC message WITHOUT trailing CRLF —
 * the caller (Server::sendToClient) appends \\r\\n automatically.
 *
 * Format: :<SERVER_NAME> <code> <nick> <message>
 */
namespace Replies
{
	std::string welcome(const std::string& nick,
	                    const std::string& user,
	                    const std::string& host);

	std::string yourHost(const std::string& nick);
	std::string created(const std::string& nick, const std::string& date);
	std::string myInfo(const std::string& nick);

	std::string errNeedMoreParams(const std::string& nick,
	                              const std::string& cmd);
	std::string errAlreadyRegistered(const std::string& nick);
	std::string errPasswdMismatch(const std::string& nick);
	std::string errNoNicknameGiven(const std::string& nick);
	std::string errErroneusNickname(const std::string& nick,
	                                const std::string& badNick);
	std::string errNicknameInUse(const std::string& nick,
	                             const std::string& usedNick);
	std::string errUnknownCommand(const std::string& nick,
	                              const std::string& cmd);
}

#endif
