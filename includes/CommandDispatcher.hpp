/* ************************************************************************** */
/*                                                                            */
/*   CommandDispatcher.hpp                                                    */
/*                                                                            */
/*   Phase 5: Command Dispatcher & Registration                               */
/*   Routes parsed IRC commands to their handler functions                    */
/*                                                                            */
/*   Owner: Ismail                                                            */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDDISPATCHER_HPP
# define COMMANDDISPATCHER_HPP

# include "ParsedCommand.hpp"
# include <string>
# include <map>

class Server;
class Client;

/**
 * @class CommandDispatcher
 * @brief Receives a ParsedCommand and calls the appropriate handler on Server.
 *
 * Phase 5 scope:
 *  - PASS — server password check
 *  - NICK — set / change nickname
 *  - USER — set username and realname
 *  - QUIT — graceful client disconnect
 *
 * Phase 7 will add:
 *  JOIN, PART, PRIVMSG, NOTICE, TOPIC, KICK, INVITE, MODE, WHO, WHOIS, …
 *
 * Orthodox Canonical Form: copy/assign disabled (owns no heap data).
 */
class CommandDispatcher
{
	public:
		/* --- Constructor / Destructor --- */
		explicit CommandDispatcher(Server& server);
		~CommandDispatcher();

		/* --- Main entry point --- */
		void dispatch(int fd, const ParsedCommand& cmd);

	private:
		/* --- Orthodox Canonical Form (disabled) --- */
		CommandDispatcher();
		CommandDispatcher(const CommandDispatcher& other);
		CommandDispatcher& operator=(const CommandDispatcher& other);

		/* --- Reference to owning server (not owned) --- */
		Server& _server;

		/* --- Registration handlers --- */
		void _handlePass(int fd, Client* client, const ParsedCommand& cmd);
		void _handleNick(int fd, Client* client, const ParsedCommand& cmd);
		void _handleUser(int fd, Client* client, const ParsedCommand& cmd);
		void _handleQuit(int fd, Client* client, const ParsedCommand& cmd);

		/* --- Helpers --- */
		bool _isValidNickname(const std::string& nick) const;
		bool _isNicknameInUse(const std::string& nick, int excludeFd) const;
		void _tryCompleteRegistration(int fd, Client* client);
};

#endif
