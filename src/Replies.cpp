#include "Replies.hpp"

static std::string _prefix(const std::string& code, const std::string& nick)
{
	return std::string(":") + SERVER_NAME + " " + code + " " + nick;
}

std::string Replies::welcome(const std::string& nick,
                             const std::string& user,
                             const std::string& host)
{
	return _prefix(RPL_WELCOME, nick)
	       + " :Welcome to the IRC Network, "
	       + nick + "!" + user + "@" + host;
}

std::string Replies::yourHost(const std::string& nick)
{
	return _prefix(RPL_YOURHOST, nick)
	       + " :Your host is " SERVER_NAME
	         ", running version ft_irc-1.0";
}

std::string Replies::created(const std::string& nick, const std::string& date)
{
	return _prefix(RPL_CREATED, nick)
	       + " :This server was created " + date;
}

std::string Replies::myInfo(const std::string& nick)
{
	return _prefix(RPL_MYINFO, nick)
	       + " " SERVER_NAME " ft_irc-1.0 o o";
}

std::string Replies::errNeedMoreParams(const std::string& nick,
                                       const std::string& cmd)
{
	return _prefix(ERR_NEEDMOREPARAMS, nick)
	       + " " + cmd + " :Not enough parameters";
}

std::string Replies::errAlreadyRegistered(const std::string& nick)
{
	return _prefix(ERR_ALREADYREGISTRED, nick)
	       + " :Unauthorized command (already registered)";
}

std::string Replies::errPasswdMismatch(const std::string& nick)
{
	return _prefix(ERR_PASSWDMISMATCH, nick)
	       + " :Password incorrect";
}

std::string Replies::errNoNicknameGiven(const std::string& nick)
{
	return _prefix(ERR_NONICKNAMEGIVEN, nick)
	       + " :No nickname given";
}

std::string Replies::errErroneusNickname(const std::string& nick,
                                          const std::string& badNick)
{
	return _prefix(ERR_ERRONEUSNICK, nick)
	       + " " + badNick + " :Erroneous nickname";
}

std::string Replies::errNicknameInUse(const std::string& nick,
                                       const std::string& usedNick)
{
	return _prefix(ERR_NICKNAMEINUSE, nick)
	       + " " + usedNick + " :Nickname is already in use";
}

std::string Replies::errUnknownCommand(const std::string& nick,
                                        const std::string& cmd)
{
	return _prefix(ERR_UNKNOWNCOMMAND, nick)
	       + " " + cmd + " :Unknown command";
}

std::string Replies::channelModeIs(const std::string& nick, const std::string& channel, const std::string& modes, const std::string& params)
{
	std::string reply = _prefix(RPL_CHANNELMODEIS, nick) + " " + channel + " " + modes;
	if (!params.empty())
		reply += " " + params;
	return reply;
}

std::string Replies::topic(const std::string& nick, const std::string& channel, const std::string& topicText)
{
	return _prefix(RPL_TOPIC, nick) + " " + channel + " :" + topicText;
}

std::string Replies::noTopic(const std::string& nick, const std::string& channel)
{
	return _prefix(RPL_NOTOPIC, nick) + " " + channel + " :No topic is set";
}

std::string Replies::inviting(const std::string& nick, const std::string& targetNick, const std::string& channel)
{
	return _prefix(RPL_INVITING, nick) + " " + targetNick + " " + channel;
}

std::string Replies::namReply(const std::string& nick, const std::string& channel, const std::string& namesList)
{
	return _prefix(RPL_NAMREPLY, nick) + " = " + channel + " :" + namesList;
}

std::string Replies::endOfNames(const std::string& nick, const std::string& channel)
{
	return _prefix(RPL_ENDOFNAMES, nick) + " " + channel + " :End of NAMES list";
}

std::string Replies::errNoSuchNick(const std::string& nick, const std::string& targetNick)
{
	return _prefix(ERR_NOSUCHNICK, nick) + " " + targetNick + " :No such nick/channel";
}

std::string Replies::errNoSuchChannel(const std::string& nick, const std::string& channelName)
{
	return _prefix(ERR_NOSUCHCHANNEL, nick) + " " + channelName + " :No such channel";
}

std::string Replies::errCannotSendToChan(const std::string& nick, const std::string& channelName)
{
	return _prefix(ERR_CANNOTSENDTOCHAN, nick) + " " + channelName + " :Cannot send to channel";
}

std::string Replies::errUserNotInChannel(const std::string& nick, const std::string& targetNick, const std::string& channelName)
{
	return _prefix(ERR_USERNOTINCHANNEL, nick) + " " + targetNick + " " + channelName + " :They aren't on that channel";
}

std::string Replies::errNotOnChannel(const std::string& nick, const std::string& channelName)
{
	return _prefix(ERR_NOTONCHANNEL, nick) + " " + channelName + " :You're not on that channel";
}

std::string Replies::errUserOnChannel(const std::string& nick, const std::string& targetNick, const std::string& channelName)
{
	return _prefix(ERR_USERONCHANNEL, nick) + " " + targetNick + " " + channelName + " :is already on channel";
}

std::string Replies::errChannelIsFull(const std::string& nick, const std::string& channelName)
{
	return _prefix(ERR_CHANNELISFULL, nick) + " " + channelName + " :Cannot join channel (+l)";
}

std::string Replies::errInviteOnlyChan(const std::string& nick, const std::string& channelName)
{
	return _prefix(ERR_INVITEONLYCHAN, nick) + " " + channelName + " :Cannot join channel (+i)";
}

std::string Replies::errBadChannelKey(const std::string& nick, const std::string& channelName)
{
	return _prefix(ERR_BADCHANNELKEY, nick) + " " + channelName + " :Cannot join channel (+k)";
}

std::string Replies::errChanOpPrivsNeeded(const std::string& nick, const std::string& channelName)
{
	return _prefix(ERR_CHANOPRIVSNEEDED, nick) + " " + channelName + " :You're not channel operator";
}
