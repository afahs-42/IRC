#include "Server.hpp"
#include "Utils.hpp"
#include <iostream>

void Server::handleUser(Client* client, const Command& cmd)
{
	if (client->isRegistered())
	{
		std::string reply = Utils::formatReply(ERR_ALREADYREGISTRED, client->getNickname(), ":You may not reregister");
		client->sendMessage(reply);
		return;
	}
	
	std::vector<std::string> params = cmd.getParams();
	if (params.size() < 3 || cmd.getTrailing().empty())
	{
		std::string reply = Utils::formatReply(ERR_NEEDMOREPARAMS, "*", "USER :Not enough parameters");
		client->sendMessage(reply);
		return;
	}
	
	if (!client->hasPassword())
	{
		std::string reply = Utils::formatReply(ERR_PASSWDMISMATCH, "*", ":Password required");
		client->sendMessage(reply);
		return;
	}
	
	if (client->getNickname().empty())
	{
		std::string reply = Utils::formatReply(ERR_NOTREGISTERED, "*", ":You must set a nickname first");
		client->sendMessage(reply);
		return;
	}
	
	client->setUsername(params[0]);
	client->setRegistered(true);
	
	std::cout << "Client " << client->getFd() << " registered as " << client->getNickname() << std::endl;
	
	sendWelcome(client);
}

void Server::sendWelcome(Client* client)
{
	std::string nick = client->getNickname();
	
	std::string welcome = Utils::formatReply(RPL_WELCOME, nick, ":Welcome to the IRC Network " + nick + "!~" + client->getUsername() + "@localhost");
	client->sendMessage(welcome);
	
	std::string yourhost = Utils::formatReply(RPL_YOURHOST, nick, ":Your host is server, running version 1.0");
	client->sendMessage(yourhost);
	
	std::string created = Utils::formatReply(RPL_CREATED, nick, ":This server was created sometime");
	client->sendMessage(created);
	
	std::string myinfo = Utils::formatReply(RPL_MYINFO, nick, "server 1.0 o o");
	client->sendMessage(myinfo);
}
