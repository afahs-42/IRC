#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>

class Client;

class Channel
{
    private:
        std::string name;
        std::string topic;
        std::string key;
        std::vector<Client*> members;
        std::vector<Client*> operators;
        std::vector<Client*> inviteList;
        bool inviteOnly;
        bool topicRestricted;
        int userLimit;
    public:
        Channel(const std::string& name);
        ~Channel();
        const std::string& getName() const;
        const std::string& getTopic() const;
        const std::string& getKey() const;
        const std::vector<Client*>& getMembers() const;
        size_t getMemberCount() const;
        void addMember(Client* client);
        void removeMember(Client* client);
        bool isMember(Client* client) const;
        void addOperator(Client* client);
        void removeOperator(Client* client);
        bool isOperator(Client* client)const;
        void addInvite(Client* client);
        bool isInvited(Client* client) const;
        void removeInvite(Client* client);
        void broadcast(const std::string& message, Client* sender);
        void broadcastToAll(const std::string& message);
        void setTopic(const std::string& newTopic);
        bool isTopicRestricted() const;
        void setTopicRestricted(bool restricted);
        void setKey(const std::string& newKey);
        void clearKey();
        bool hasKey() const;
        bool checkKey(const std::string& providedKey) const;
        void setUserLimit(int limit);
        int getUserLimit() const;
        bool isFull() const;
        void setInviteOnly(bool inviteOnly);
        bool isInviteOnly() const;
        bool isEmpty() const;
        std::string getMemberList() const;
};

#endif