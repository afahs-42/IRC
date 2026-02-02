#include "Server.hpp"
#include "Utils.hpp"
#include <iostream>
#include <cstdlib>

void Server::handleMode(Client* client, const Command& cmd)
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
		                                       "MODE :Not enough parameters");
		client->sendMessage(reply);
		return;
	}

	std::string target = params[0];

	if (!Utils::isChannelName(target))
	{
		return;
	}

	Channel* channel = getChannel(target);
	if (!channel)
	{
		std::string reply = Utils::formatReply(ERR_NOSUCHCHANNEL, client->getNickname(),
		                                       target + " :No such channel");
		client->sendMessage(reply);
		return;
	}

	if (!channel->isMember(client))
	{
		std::string reply = Utils::formatReply(ERR_NOTONCHANNEL, client->getNickname(),
		                                       target + " :You're not on that channel");
		client->sendMessage(reply);
		return;
	}

	if (params.size() == 1)
	{
		std::string modeString = "+";
		std::string modeParams = "";

		if (channel->isInviteOnly())
			modeString += "i";
		if (channel->isTopicRestricted())
			modeString += "t";
		if (channel->hasKey())
		{
			modeString += "k";
		}
		if (channel->getUserLimit() > 0)
		{
			modeString += "l";
			modeParams += " " + Utils::intToString(channel->getUserLimit());
		}

		std::string reply = ":server 324 " + client->getNickname() + " " + 
		                    target + " " + modeString + modeParams;
		client->sendMessage(reply);
		return;
	}

	if (!channel->isOperator(client))
	{
		std::string reply = Utils::formatReply(ERR_CHANOPRIVNEEDED, client->getNickname(),
		                                       target + " :You're not channel operator");
		client->sendMessage(reply);
		return;
	}

	std::string modeStr = params[1];
	std::vector<std::string> modeParams;

	for (size_t i = 2; i < params.size(); ++i)
		modeParams.push_back(params[i]);

	bool adding = true;
	size_t paramIndex = 0;
	std::string appliedModes = "";
	std::string appliedParams = "";

	for (size_t i = 0; i < modeStr.length(); ++i)
	{
		char mode = modeStr[i];

		if (mode == '+')
		{
			adding = true;
			continue;
		}
		else if (mode == '-')
		{
			adding = false;
			continue;
		}

		switch (mode)
		{
			case 'i':
			{
				channel->setInviteOnly(adding);
				appliedModes += (adding ? "+" : "-");
				appliedModes += "i";
				std::cout << client->getNickname() << (adding ? " set +i" : " removed +i") 
				          << " on " << target << std::endl;
				break;
			}

			case 't':
			{
				channel->setTopicRestricted(adding);
				appliedModes += (adding ? "+" : "-");
				appliedModes += "t";
				std::cout << client->getNickname() << (adding ? " set +t" : " removed +t") 
				          << " on " << target << std::endl;
				break;
			}

			case 'k':
			{
				if (adding)
				{
					if (paramIndex < modeParams.size())
					{
						std::string key = modeParams[paramIndex++];
						channel->setKey(key);
						appliedModes += "+k";
						appliedParams += " " + key;
						std::cout << client->getNickname() << " set key on " << target << std::endl;
					}
				}
				else
				{
					channel->clearKey();
					appliedModes += "-k";
					if (paramIndex < modeParams.size())
					{
						appliedParams += " " + modeParams[paramIndex++];
					}
					std::cout << client->getNickname() << " removed key from " << target << std::endl;
				}
				break;
			}

			case 'o':
			{
				if (paramIndex < modeParams.size())
				{
					std::string targetNick = modeParams[paramIndex++];
					Client* targetClient = getClientByNickname(targetNick);

					if (!targetClient)
					{
						std::string reply = Utils::formatReply(ERR_NOSUCHNICK, client->getNickname(),
						                                       targetNick + " :No such nick/channel");
						client->sendMessage(reply);
						continue;
					}

					if (!channel->isMember(targetClient))
					{
						std::string reply = Utils::formatReply(ERR_USERNOTINCHANNEL, client->getNickname(),
						                                       targetNick + " " + target + " :They aren't on that channel");
						client->sendMessage(reply);
						continue;
					}

					if (adding)
						channel->addOperator(targetClient);
					else
						channel->removeOperator(targetClient);

					appliedModes += (adding ? "+" : "-");
					appliedModes += "o";
					appliedParams += " " + targetNick;
					std::cout << client->getNickname() << (adding ? " gave operator to " : " removed operator from ")
					          << targetNick << " in " << target << std::endl;
				}
				break;
			}

			case 'l':
			{
				if (adding)
				{
					if (paramIndex < modeParams.size())
					{
						std::string limitStr = modeParams[paramIndex++];
						int limit = std::atoi(limitStr.c_str());

						if (limit > 0)
						{
							channel->setUserLimit(limit);
							appliedModes += "+l";
							appliedParams += " " + limitStr;
							std::cout << client->getNickname() << " set limit to " << limit 
							          << " on " << target << std::endl;
						}
					}
				}
				else
				{
					channel->setUserLimit(0); 
					appliedModes += "-l";
					std::cout << client->getNickname() << " removed limit from " << target << std::endl;
				}
				break;
			}

			default:
			{
				std::string unknownMode(1, mode);
				std::string reply = ":server 472 " + client->getNickname() + " " + 
				                    unknownMode + " :is unknown mode char to me";
				client->sendMessage(reply);
				std::cerr << "Unknown mode: " << mode << std::endl;
				break;
			}
		}
	}

	if (!appliedModes.empty())
	{
		std::string modeMsg = Utils::formatMessage(
			client->getNickname() + "!~" + client->getUsername() + "@localhost",
			"MODE",
			target + " " + appliedModes + appliedParams
		);
		channel->broadcastToAll(modeMsg);
	}
}
