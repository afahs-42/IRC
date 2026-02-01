#include "Server.hpp"
#include "Utils.hpp"
#include <iostream>

void Server::handleTopic(Client* client, const Command& cmd)
{
	if (!client->isRegistered())
	{
		std::string reply = Utils::formatReply(ERR_NOTREGISTERED, "*", ":You have not registered");
		client->sendMessage(reply);
		return;
	}
	std::vector<std::string> params = cmd.getParams();
	if (params.empty())
	{
		std::string reply = Utils::formatReply(ERR_NEEDMOREPARAMS, client->getNickname(),
		                                       "TOPIC :Not enough parameters");
		client->sendMessage(reply);
		return;
	}
	std::string channelName = params[0];
	Channel* channel = getChannel(channelName);
	if (!channel)
	{
		std::string reply = Utils::formatReply(ERR_NOSUCHCHANNEL, client->getNickname(),
		                                       channelName + " :No such channel");
		client->sendMessage(reply);
		return;
	}
	if (!channel->isMember(client))
	{
		std::string reply = Utils::formatReply(ERR_NOTONCHANNEL, client->getNickname(),
		                                       channelName + " :You're not on that channel");
		client->sendMessage(reply);
		return;
	}
	if (params.size() == 1 && cmd.getTrailing().empty())
	{
		if (channel->getTopic().empty())
		{
			std::string reply = Utils::formatReply(RPL_NOTOPIC, client->getNickname(),
			                                       channelName + " :No topic is set");
			client->sendMessage(reply);
		}
		else
		{
			std::string reply = Utils::formatReply(RPL_TOPIC, client->getNickname(),
			                                       channelName + " :" + channel->getTopic());
			client->sendMessage(reply);
		}
		return;
	}
	if (channel->isTopicRestricted() && !channel->isOperator(client))
	{
		std::string reply = Utils::formatReply(ERR_CHANOPRIVNEEDED, client->getNickname(),
		                                       channelName + " :You're not channel operator");
		client->sendMessage(reply);
		return;
	}
	std::string newTopic = cmd.getTrailing();
	channel->setTopic(newTopic);
	std::string topicMsg = Utils::formatMessage(
		client->getNickname() + "!~" + client->getUsername() + "@localhost",
		"TOPIC",
		channelName + " :" + newTopic
	);
	channel->broadcastToAll(topicMsg);
	if (newTopic.empty())
		std::cout << client->getNickname() << " cleared topic in " << channelName << std::endl;
	else
		std::cout << client->getNickname() << " set topic in " << channelName << " to: " << newTopic << std::endl;
}
