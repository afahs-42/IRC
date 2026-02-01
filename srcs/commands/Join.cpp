#include "Server.hpp"
#include "Utils.hpp"
#include <iostream>
#include <sstream>

void Server::handleJoin(Client* client, const Command& cmd)
{
	if (!client->isRegistered())
	{
		client->sendMessage(Utils::formatReply(ERR_NOTREGISTERED, "*", ":You have not registered"));
		return;
	}
	std::vector<std::string> params = cmd.getParams();
	if (params.empty())
	{
		client->sendMessage(Utils::formatReply(ERR_NEEDMOREPARAMS,
			client->getNickname(), "JOIN :Not enough parameters"));
		return;
	}
	if (params[0] == "0")
	{
		handlePartAll(client);
		return;
	}
	std::vector<std::string> channels = Utils::splitByComma(params[0]);
	std::vector<std::string> keys;
	if (params.size() > 1)
		keys = Utils::splitByComma(params[1]);
	for (size_t i = 0; i < channels.size(); ++i)
	{
		std::string channelName = channels[i];
		std::string key = (i < keys.size()) ? keys[i] : "";
		if (!Utils::isValidChannelName(channelName))
		{
			client->sendMessage(Utils::formatReply(ERR_BADCHANMASK,
				client->getNickname(), channelName + " :Bad Channel Mask"));
			continue;
		}
		Channel* channel = getChannel(channelName);
		bool isNewChannel = false;
		if (!channel)
		{
			channel = createChannel(channelName);
			isNewChannel = true;
		}
		if (channel->isMember(client))
			continue;
		if (!isNewChannel)
		{
			if (channel->isInviteOnly() && !channel->isInvited(client))
			{
				client->sendMessage(Utils::formatReply(ERR_INVITEONLYCHAN,
					client->getNickname(), channelName + " :Cannot join channel (+i)"));
				continue;
			}
			if (channel->hasKey() && !channel->checkKey(key))
			{
				client->sendMessage(Utils::formatReply(ERR_BADCHANNELKEY,
					client->getNickname(), channelName + " :Cannot join channel (+k)"));
				continue;
			}
			if (channel->isFull())
			{
				client->sendMessage(Utils::formatReply(ERR_CHANNELISFULL,
					client->getNickname(), channelName + " :Cannot join channel (+l)"));
				continue;
			}
		}
		channel->addMember(client);
		if (channel->isInvited(client))
			channel->removeInvite(client);
		std::string joinMsg = Utils::formatMessage(
			client->getNickname() + "!~" + client->getUsername() + "@localhost",
			"JOIN", channelName);
		channel->broadcastToAll(joinMsg);
		if (!channel->getTopic().empty())
			client->sendMessage(Utils::formatReply(RPL_TOPIC,
				client->getNickname(), channelName + " :" + channel->getTopic()));
		else
			client->sendMessage(Utils::formatReply(RPL_NOTOPIC,
				client->getNickname(), channelName + " :No topic is set"));
		std::string namesReply = Utils::formatReply(RPL_NAMREPLY,
			client->getNickname(), "= " + channelName + " :" + channel->getMemberList());
		client->sendMessage(namesReply);
		client->sendMessage(Utils::formatReply(RPL_ENDOFNAMES,
			client->getNickname(), channelName + " :End of /NAMES list"));
	}
}

void Server::handlePartAll(Client* client)
{
	std::vector<Channel*> clientChannels;

	for (size_t i = 0; i < _channels.size(); ++i)
	{
		if (_channels[i]->isMember(client))
			clientChannels.push_back(_channels[i]);
	}
	for (size_t i = 0; i < clientChannels.size(); ++i)
	{
		Channel* channel = clientChannels[i];

		std::string partMsg = Utils::formatMessage(
			client->getNickname() + "!~" + client->getUsername() + "@localhost",
			"PART", channel->getName() + " :Left all channels");
		channel->broadcastToAll(partMsg);
		channel->removeMember(client);
		if (channel->isEmpty())
			removeChannel(channel->getName());
	}
}