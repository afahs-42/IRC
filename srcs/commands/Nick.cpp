#include "Server.hpp"
#include "Utils.hpp"
#include <iostream>

void Server::handleNick(Client* client, const Command& cmd)
{
	std::vector<std::string> params = cmd.getParams();
	if (params.empty())
	{
		std::string reply = Utils::formatReply(ERR_NONICKNAMEGIVEN, "*", ":No nickname given");
		client->sendMessage(reply);
		return;
	}
	
	std::string nickname = params[0];
	
	if (nickname.empty() || nickname.length() > 9)
	{
		std::string reply = Utils::formatReply(ERR_ERRONEUSNICKNAME, "*", nickname + " :Erroneous nickname");
		client->sendMessage(reply);
		return;
	}
	
	if (isNicknameInUse(nickname, client))
	{
		std::string reply = Utils::formatReply(ERR_NICKNAMEINUSE, "*", nickname + " :Nickname is already in use");
		client->sendMessage(reply);
		return;
	}
	
	std::string oldNick = client->getNickname();
	client->setNickname(nickname);
	
	if (!oldNick.empty() && client->isRegistered())
	{
		std::string msg = Utils::formatMessage(oldNick + "!~" + client->getUsername() + "@localhost", "NICK", ":" + nickname);
		client->sendMessage(msg);
	}
	
	std::cout << "Client " << client->getFd() << " set nickname to: " << nickname << std::endl;
}
