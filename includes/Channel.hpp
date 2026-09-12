#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <map>
# include <vector>

class Client;

class Channel
{
	public:
		Channel(const std::string& name);
		~Channel();

		const std::string&					getName()  const;
		const std::string&					getTopic() const;
		const std::map<int, Client*>&		getMembers() const;

		void		addMember(Client* client);
		void		removeMember(int fd);
		bool		hasMember(int fd) const;
		Client*		getMember(int fd) const;

		void		addOperator(int fd);
		void		removeOperator(int fd);
		bool		isOperator(int fd) const;

		void		setTopic(const std::string& topic);

		bool				isInviteOnly() const;
		void				setInviteOnly(bool state);

		bool				isTopicRestricted() const;
		void				setTopicRestricted(bool state);

		const std::string&	getKey() const;
		void				setKey(const std::string& key);

		int					getLimit() const;
		void				setLimit(int limit);

		void				addInvite(int fd);
		void				removeInvite(int fd);
		bool				isInvited(int fd) const;

		bool				isFull() const;

	private:
		Channel();
		Channel(const Channel& other);
		Channel& operator=(const Channel& other);

		std::string					_name;
		std::string					_topic;
		std::map<int, Client*>		_members;
		std::vector<int>			_operators;

		bool						_inviteOnly;
		bool						_topicRestricted;
		std::string					_key;
		int							_limit;
		std::vector<int>			_invitedFds;
};

#endif
