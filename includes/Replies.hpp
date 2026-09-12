#ifndef REPLIES_HPP
# define REPLIES_HPP

# include <string>

# define RPL_WELCOME          "001"
# define RPL_YOURHOST         "002"
# define RPL_CREATED          "003"
# define RPL_MYINFO           "004"
# define RPL_CHANNELMODEIS    "324"
# define RPL_NOTOPIC          "331"
# define RPL_TOPIC            "332"
# define RPL_INVITING         "341"
# define RPL_NAMREPLY         "353"
# define RPL_ENDOFNAMES       "366"

# define ERR_NOSUCHNICK       "401"
# define ERR_NOSUCHCHANNEL    "403"
# define ERR_CANNOTSENDTOCHAN "404"
# define ERR_UNKNOWNCOMMAND   "421"
# define ERR_NONICKNAMEGIVEN  "431"
# define ERR_ERRONEUSNICK     "432"
# define ERR_NICKNAMEINUSE    "433"
# define ERR_USERNOTINCHANNEL "441"
# define ERR_NOTONCHANNEL     "442"
# define ERR_USERONCHANNEL    "443"
# define ERR_NEEDMOREPARAMS   "461"
# define ERR_ALREADYREGISTRED "462"
# define ERR_PASSWDMISMATCH   "464"
# define ERR_CHANNELISFULL    "471"
# define ERR_INVITEONLYCHAN   "473"
# define ERR_BADCHANNELKEY    "475"
# define ERR_CHANOPRIVSNEEDED "482"

# define SERVER_NAME          "ircserv"

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

	std::string channelModeIs(const std::string& nick, const std::string& channel, const std::string& modes, const std::string& params);
	std::string topic(const std::string& nick, const std::string& channel, const std::string& topicText);
	std::string noTopic(const std::string& nick, const std::string& channel);
	std::string inviting(const std::string& nick, const std::string& targetNick, const std::string& channel);
	std::string namReply(const std::string& nick, const std::string& channel, const std::string& namesList);
	std::string endOfNames(const std::string& nick, const std::string& channel);

	std::string errNoSuchNick(const std::string& nick, const std::string& targetNick);
	std::string errNoSuchChannel(const std::string& nick, const std::string& channelName);
	std::string errCannotSendToChan(const std::string& nick, const std::string& channelName);
	std::string errUserNotInChannel(const std::string& nick, const std::string& targetNick, const std::string& channelName);
	std::string errNotOnChannel(const std::string& nick, const std::string& channelName);
	std::string errUserOnChannel(const std::string& nick, const std::string& targetNick, const std::string& channelName);
	std::string errChannelIsFull(const std::string& nick, const std::string& channelName);
	std::string errInviteOnlyChan(const std::string& nick, const std::string& channelName);
	std::string errBadChannelKey(const std::string& nick, const std::string& channelName);
	std::string errChanOpPrivsNeeded(const std::string& nick, const std::string& channelName);
}

#endif
