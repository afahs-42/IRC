#include "Server.hpp"
#include "Utils.hpp"
#include <iostream>

void Server::handlePart(Client* client, const Command& cmd)
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
		                                       "PART :Not enough parameters");
		client->sendMessage(reply);
		return;
	}

	std::vector<std::string> channels = Utils::splitByComma(params[0]);
	std::string reason = cmd.getTrailing().empty() ? "Leaving" : cmd.getTrailing();

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

		std::string partMsg = Utils::formatMessage(
			client->getNickname() + "!~" + client->getUsername() + "@localhost",
			"PART",
			channelName + " :" + reason
		);

		channel->broadcastToAll(partMsg);
		channel->removeMember(client);

		std::cout << client->getNickname() << " left " << channelName << std::endl;

		if (channel->isEmpty())
			removeChannel(channelName);
	}
}
