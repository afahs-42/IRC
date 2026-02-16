#include "Server.hpp"
#include "Utils.hpp"
#include <iostream>
#include <unistd.h>

void Server::handleQuit(Client* client, const Command& cmd)
{
	std::string reason = cmd.getTrailing().empty() ? "Client quit" : cmd.getTrailing();

	std::string quitMsg = Utils::formatMessage(
		client->getNickname() + "!~" + client->getUsername() + "@localhost",
		"QUIT",
		":" + reason
	);

	for (size_t i = 0; i < _channels.size(); ++i)
	{
		if (_channels[i]->isMember(client))
		{
			_channels[i]->broadcast(quitMsg, NULL);
			_channels[i]->removeMember(client);
			
			if (_channels[i]->isEmpty())
			{
				std::string channelName = _channels[i]->getName();
				removeChannel(channelName);
				--i;
			}
		}
	}

	std::cout << "Client " << client->getFd() << " quit: " << reason << std::endl;

	int fd = client->getFd();
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i] == client)
		{
			close(fd);
			delete client;
			_clients.erase(_clients.begin() + i);
			_fds.erase(_fds.begin() + (i + 1));
			break;
		}
	}
}
