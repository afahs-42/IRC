#include "Channel.hpp"
#include "Client.hpp"
#include <algorithm>
#include <sstream>

Channel::Channel(const std::string& name)
    : name(name), 
      topic(""),
      key(""),
      inviteOnly(false),
      topicRestricted(true),
      userLimit(0)
{}

Channel::~Channel()
{
    members.clear();
    operators.clear();
    inviteList.clear();
}

const std::string& Channel::getName() const
{
    return name;
}

const std::string& Channel::getTopic() const
{
    return topic;
}

const std::string& Channel::getKey() const
{
    return key;
}

const std::vector<Client*>& Channel::getMembers() const
{
    return members;
}


size_t Channel::getMemberCount() const
{
    return members.size();
}

void Channel::addMember(Client* client)
{
    if (!isMember(client))
    {
        members.push_back(client);
        if (members.size() == 1)
            addOperator(client);
    }
}

void Channel::removeMember(Client* client)
{
    members.erase(std::remove(members.begin(), members.end(), client), members.end());
    operators.erase(std::remove(operators.begin(), operators.end(), client), operators.end());
    inviteList.erase(std::remove(inviteList.begin(), inviteList.end(), client), inviteList.end());
}

bool Channel::isMember(Client* client) const
{
    return std::find(members.begin(), members.end(), client) != members.end();
}

void Channel::addOperator(Client* client)
{
    if (isMember(client) && !isOperator(client))
        operators.push_back(client);
}

void Channel::removeOperator(Client* client)
{
    operators.erase(std::remove(operators.begin(), operators.end(), client), operators.end());
}

bool Channel::isOperator(Client* client) const
{
    return std::find(operators.begin(), operators.end(), client) != operators.end();
}

void Channel::addInvite(Client* client)
{
    if (!isInvited(client))
        inviteList.push_back(client);
}

bool Channel::isInvited(Client* client) const
{
    return std::find(inviteList.begin(), inviteList.end(), client) != inviteList.end();
}

void Channel::removeInvite(Client* client)
{
    inviteList.erase(std::remove(inviteList.begin(), inviteList.end(), client), inviteList.end());
}

void Channel::broadcast(const std::string& message, Client* sender)
{
    for (size_t i = 0; i < members.size(); ++i)
    {
        if (members[i] != sender)
            members[i]->sendMessage(message);
    }
}

 void Channel::broadcastToAll(const std::string& message)
{
    broadcast(message, NULL);
}

void Channel::setTopic(const std::string& newTopic)
{
    topic = newTopic;
}

bool Channel::isTopicRestricted() const
{
    return topicRestricted;
}

void Channel::setTopicRestricted(bool restricted)
{
    topicRestricted = restricted;
}

void Channel::setKey(const std::string& newKey)
{
    key = newKey;
}

void Channel::clearKey()
{
    key.clear();
}

bool Channel::hasKey() const
{
    return !key.empty();
}

bool Channel::checkKey(const std::string& providedKey) const
{
    if (!hasKey())
        return true;
    return key == providedKey;
}

void Channel::setUserLimit(int limit)
{
    userLimit = limit;
}

int Channel::getUserLimit() const
{
    return userLimit;
}

bool Channel::isFull() const
{
    if (userLimit <= 0)
        return false;
    return static_cast<int>(members.size()) >= userLimit;
}

void Channel::setInviteOnly(bool invite)
{
    inviteOnly = invite;
}

bool Channel::isInviteOnly() const
{
    return inviteOnly;
}

bool Channel::isEmpty() const
{
    return members.empty();
}

std::string Channel::getMemberList() const
{
    std::string list;
    
    for (size_t i = 0; i < members.size(); ++i)
    {
        if (isOperator(members[i]))
            list += "@";
        
        list += members[i]->getNickname();
        
        if (i < members.size() - 1)
            list += " ";
    }
    return list;
}