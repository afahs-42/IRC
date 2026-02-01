#include "Server.hpp"
#include "Utils.hpp"
#include <iostream>

void Server::handleKick(Client* client, const Command& cmd)
{
	if (!client->isRegistered())
	{
		std::string reply = Utils::formatReply(ERR_NOTREGISTERED, "*", ":You have not registered");
		client->sendMessage(reply);
		return;
	}
	std::vector<std::string> params = cmd.getParams();
	if (params.size() < 2)
	{
		std::string reply = Utils::formatReply(ERR_NEEDMOREPARAMS, client->getNickname(),
		                                       "KICK :Not enough parameters");
		client->sendMessage(reply);
		return;
	}
	std::vector<std::string> channels = Utils::splitByComma(params[0]);
	std::vector<std::string> users = Utils::splitByComma(params[1]);
	std::string reason = cmd.getTrailing().empty() ? "You have been kicked" : cmd.getTrailing();
	for (size_t i = 0; i < channels.size(); ++i)
	{
		std::string channelName = channels[i];
		Channel* channel = getChannel(channelName);
		if (!channel)
		{
			std::string reply = Utils::formatReply(ERR_NOSUCHCHANNEL, client->getNickname(),
			                                       channelName + " :No such channel");
			client->sendMessage(reply);
			continue;
		}
		if (!channel->isMember(client))
		{
			std::string reply = Utils::formatReply(ERR_NOTONCHANNEL, client->getNickname(),
			                                       channelName + " :You're not on that channel");
			client->sendMessage(reply);
			continue;
		}
		if (!channel->isOperator(client))
		{
			std::string reply = Utils::formatReply(ERR_CHANOPRIVNEEDED, client->getNickname(),
			                                       channelName + " :You're not channel operator");
			client->sendMessage(reply);
			continue;
		}
		for (size_t j = 0; j < users.size(); ++j)
		{
			std::string targetNick = users[j];
			Client* target = getClientByNickname(targetNick);
			if (!target)
			{
				std::string reply = Utils::formatReply(ERR_NOSUCHNICK, client->getNickname(),
				                                       targetNick + " :No such nick/channel");
				client->sendMessage(reply);
				continue;
			}
			if (!channel->isMember(target))
			{
				std::string reply = Utils::formatReply(ERR_USERNOTINCHANNEL, client->getNickname(),
				                                       targetNick + " " + channelName + " :They aren't on that channel");
				client->sendMessage(reply);
				continue;
			}
			if (target == client)
				continue;
			std::string kickMsg = Utils::formatMessage(
				client->getNickname() + "!~" + client->getUsername() + "@localhost",
				"KICK",
				channelName + " " + targetNick + " :" + reason
			);
			channel->broadcastToAll(kickMsg);
			channel->removeMember(target);
			std::cout << client->getNickname() << " kicked " << targetNick
			          << " from " << channelName << std::endl;
			if (channel->isEmpty())
			{
				removeChannel(channelName);
				break;
			}
		}
	}
}
