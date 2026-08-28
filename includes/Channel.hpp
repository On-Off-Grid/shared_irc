/* ************************************************************************** */
/*                                                                            */
/*   Channel.hpp                                                              */
/*                                                                            */
/*   Phase 5: Command Dispatcher & Registration                               */
/*   Channel class — manages IRC channel membership and messages              */
/*                                                                            */
/*   Owner: Ismail                                                            */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <map>
# include <vector>

class Client;

/**
 * @class Channel
 * @brief Represents an IRC channel (e.g. #general).
 *
 * Phase 5 scope (skeleton):
 *  - Stores channel name and optional topic.
 *  - Tracks members (fd → Client*).
 *  - Tracks operators.
 *  - Broadcasts messages to all members.
 *
 * Phase 7 will expand MODE, KICK, INVITE, TOPIC enforcement.
 *
 * Orthodox Canonical Form:
 *  - Parameterised constructor (name)
 *  - Destructor
 *  - Copy constructor  (private — disabled)
 *  - Assignment op     (private — disabled)
 */
class Channel
{
	public:
		/* --- Constructor / Destructor --- */
		Channel(const std::string& name);
		~Channel();

		/* --- Getters --- */
		const std::string&					getName()  const;
		const std::string&					getTopic() const;
		const std::map<int, Client*>&		getMembers() const;

		/* --- Member management --- */
		void		addMember(Client* client);
		void		removeMember(int fd);
		bool		hasMember(int fd) const;
		Client*		getMember(int fd) const;

		/* --- Operator management --- */
		void		addOperator(int fd);
		void		removeOperator(int fd);
		bool		isOperator(int fd) const;

		/* --- Topic --- */
		void		setTopic(const std::string& topic);

		/* --- Broadcast --- */
		void		broadcast(const std::string& message, int excludeFd = -1) const;

		/* --- Mode Getters / Setters --- */
		bool				isInviteOnly() const;
		void				setInviteOnly(bool state);

		bool				isTopicRestricted() const;
		void				setTopicRestricted(bool state);

		const std::string&	getKey() const;
		void				setKey(const std::string& key);

		int					getLimit() const;
		void				setLimit(int limit);

		/* --- Invite Management --- */
		void				addInvite(int fd);
		void				removeInvite(int fd);
		bool				isInvited(int fd) const;

		/* --- Helpers --- */
		bool				isFull() const;

	private:
		/* --- Orthodox Canonical Form (disabled) --- */
		Channel();
		Channel(const Channel& other);
		Channel& operator=(const Channel& other);

		/* --- Attributes --- */
		std::string					_name;
		std::string					_topic;
		std::map<int, Client*>		_members;
		std::vector<int>			_operators;

		/* --- Phase 6: Mode and invite flags --- */
		bool						_inviteOnly;
		bool						_topicRestricted;
		std::string					_key;
		int							_limit;
		std::vector<int>			_invitedFds;
};

#endif
