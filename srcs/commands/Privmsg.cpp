#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"
#include "Utils.hpp"
#include <iostream>

void Server::handlePrivmsg(Client* client, const Command& cmd)
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
		std::string reply = Utils::formatReply(ERR_NORECIPIENT, client->getNickname(), 
		                                       ":No recipient given (PRIVMSG)");
		client->sendMessage(reply);
		return;
	}
	if (cmd.getTrailing().empty())
	{
		std::string reply = Utils::formatReply(ERR_NOTEXTTOSEND, client->getNickname(), 
		                                       ":No text to send");
		client->sendMessage(reply);
		return;
	}
	std::string message = cmd.getTrailing();
	std::vector<std::string> targets = Utils::splitByComma(params[0]);
	if (targets.size() > 10)
	{
		std::string reply = Utils::formatReply(ERR_TOOMANYTARGETS, client->getNickname(), 
		                                       params[0] + " :Too many recipients");
		client->sendMessage(reply);
		return;
	}
	for (size_t i = 0; i < targets.size(); ++i)
	{
		std::string target = targets[i];

		if (Utils::isChannelName(target))
		{
			handleChannelMessage(client, target, message);
		}
		else
		{
			handlePrivateMessage(client, target, message);
		}
	}
}

void Server::handleChannelMessage(Client* client, const std::string& channelName, 
                                  const std::string& message)
{
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
		std::string reply = Utils::formatReply(ERR_CANNOTSENDTOCHAN, client->getNickname(), 
		                                       channelName + " :Cannot send to channel");
		client->sendMessage(reply);
		return;
	}
	std::string fullMessage = Utils::formatMessage(client->getNickname() + "!~" + 
	                                              client->getUsername() + "@localhost", 
	                                              "PRIVMSG", 
	                                              channelName + " :" + message);
	channel->broadcast(fullMessage, client);
	std::cout << client->getNickname() << " -> " << channelName 
	          << ": " << message << std::endl;
}

void Server::handlePrivateMessage(Client* sender, const std::string& targetNick, 
                                  const std::string& message)
{
	Client* target = getClientByNickname(targetNick);
	
	if (!target)
	{
		std::string reply = Utils::formatReply(ERR_NOSUCHNICK, sender->getNickname(), 
		                                       targetNick + " :No such nick/channel");
		sender->sendMessage(reply);
		return;
	}
	std::string fullMessage = Utils::formatMessage(sender->getNickname() + "!~" + 
	                                              sender->getUsername() + "@localhost", 
	                                              "PRIVMSG", 
	                                              targetNick + " :" + message);
	target->sendMessage(fullMessage);
	std::cout << sender->getNickname() << " -> " << targetNick 
	          << " (PM): " << message << std::endl;
}

void Server::handleNotice(Client* client, const Command& cmd)
{
	if (!client->isRegistered())
		return;
	std::vector<std::string> params = cmd.getParams();
	if (params.empty() || cmd.getTrailing().empty())
		return;
	std::string message = cmd.getTrailing();
	std::vector<std::string> targets = Utils::splitByComma(params[0]);
	if (targets.size() > 10)
		return;
	for (size_t i = 0; i < targets.size(); ++i)
	{
		std::string target = targets[i];

		if (Utils::isChannelName(target))
		{
			Channel* channel = getChannel(target);
			if (!channel || !channel->isMember(client))
				continue;
			std::string fullMessage = Utils::formatMessage(client->getNickname() + "!~" + 
			                                              client->getUsername() + "@localhost", 
			                                              "NOTICE", 
			                                              target + " :" + message);
			channel->broadcast(fullMessage, client);
		}
		else
		{
			Client* targetClient = getClientByNickname(target);
			if (!targetClient)
				continue;
			std::string fullMessage = Utils::formatMessage(client->getNickname() + "!~" + 
			                                              client->getUsername() + "@localhost", 
			                                              "NOTICE", 
			                                              target + " :" + message);
			targetClient->sendMessage(fullMessage);
		}
	}
}