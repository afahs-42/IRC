#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"
#include "Utils.hpp"
#include <iostream>

void Server::handleInvite(Client* client, const Command& cmd)
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
		                                       "INVITE :Not enough parameters");
		client->sendMessage(reply);
		return;
	}
	std::string targetNick = params[0];
	std::string channelName = params[1];
	Client* target = getClientByNickname(targetNick);
	if (!target)
	{
		std::string reply = Utils::formatReply(ERR_NOSUCHNICK, client->getNickname(),
		                                       targetNick + " :No such nick/channel");
		client->sendMessage(reply);
		return;
	}
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
	if (channel->isInviteOnly() && !channel->isOperator(client))
	{
		std::string reply = Utils::formatReply(ERR_CHANOPRIVNEEDED, client->getNickname(),
		                                       channelName + " :You're not channel operator");
		client->sendMessage(reply);
		return;
	}
	if (channel->isMember(target))
	{
		std::string reply = Utils::formatReply(RPL_INVITME, client->getNickname(),
		                                       channelName + " :' is not joined to that channel");
		client->sendMessage(reply);
		return;
	}
	channel->addInvite(target);
	std::string confirmReply = Utils::formatReply(RPL_INVITING, client->getNickname(),
	                                              targetNick + " :" + channelName);
	client->sendMessage(confirmReply);
	std::string inviteMsg = Utils::formatMessage(
		client->getNickname() + "!~" + client->getUsername() + "@localhost",
		"INVITE",
		targetNick + " :" + channelName
	);
	target->sendMessage(inviteMsg);
	std::cout << client->getNickname() << " invited " << targetNick
	          << " to " << channelName << std::endl;
}
