#include "Server.hpp"
#include "Utils.hpp"
#include <iostream>

void Server::handlePass(Client* client, const Command& cmd)
{
	if (client->isRegistered())
	{
		std::string reply = Utils::formatReply(ERR_ALREADYREGISTRED, client->getNickname(), ":You may not reregister");
		client->sendMessage(reply);
		return;
	}
	
	std::vector<std::string> params = cmd.getParams();
	if (params.empty())
	{
		std::string reply = Utils::formatReply(ERR_NEEDMOREPARAMS, "*", "PASS :Not enough parameters");
		client->sendMessage(reply);
		return;
	}
	
	if (params[0] == getPassword())
	{
		client->setHasPassword(true);
		std::cout << "Client " << client->getFd() << " authenticated with password" << std::endl;
	}
	else
	{
		std::string reply = Utils::formatReply(ERR_PASSWDMISMATCH, "*", ":Password incorrect");
		client->sendMessage(reply);
	}
}
