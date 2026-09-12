#ifndef COMMANDDISPATCHER_HPP
# define COMMANDDISPATCHER_HPP

# include "ParsedCommand.hpp"
# include <string>
# include <map>

class Server;
class Client;

class CommandDispatcher
{
	public:
		explicit CommandDispatcher(Server& server);
		~CommandDispatcher();

		void dispatch(int fd, const ParsedCommand& cmd);

	private:
		CommandDispatcher();
		CommandDispatcher(const CommandDispatcher& other);
		CommandDispatcher& operator=(const CommandDispatcher& other);

		Server& _server;

		void _handlePass(int fd, Client* client, const ParsedCommand& cmd);
		void _handleNick(int fd, Client* client, const ParsedCommand& cmd);
		void _handleUser(int fd, Client* client, const ParsedCommand& cmd);
		void _handleQuit(int fd, Client* client, const ParsedCommand& cmd);

		void _handleJoin(int fd, Client* client, const ParsedCommand& cmd);
		void _handlePart(int fd, Client* client, const ParsedCommand& cmd);
		void _handlePrivmsg(int fd, Client* client, const ParsedCommand& cmd);
		void _handleNotice(int fd, Client* client, const ParsedCommand& cmd);
		void _handleTopic(int fd, Client* client, const ParsedCommand& cmd);
		void _handleKick(int fd, Client* client, const ParsedCommand& cmd);
		void _handleInvite(int fd, Client* client, const ParsedCommand& cmd);
		void _handleMode(int fd, Client* client, const ParsedCommand& cmd);

		bool _isValidNickname(const std::string& nick) const;
		bool _isNicknameInUse(const std::string& nick, int excludeFd) const;
		void _tryCompleteRegistration(int fd, Client* client);
};

#endif
