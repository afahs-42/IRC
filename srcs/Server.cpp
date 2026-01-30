#include "Server.hpp"
#include "Parser.hpp"
#include "Utils.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

Server::Server(int port, const std::string& password) : _serverFd(-1), _port(port), _password(password)
{
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd < 0)
		throw std::runtime_error("Failed to create socket");

	int opt = 1;
	if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		close(_serverFd);
		throw std::runtime_error("Failed to set socket options");
	}

	if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(_serverFd);
		throw std::runtime_error("Failed to set non-blocking mode");
	}

	struct sockaddr_in serverAddr;
	std::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(_port);

	if (bind(_serverFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
	{
		close(_serverFd);
		throw std::runtime_error("Failed to bind socket");
	}

	if (listen(_serverFd, 10) < 0)
	{
		close(_serverFd);
		throw std::runtime_error("Failed to listen on socket");
	}

	struct pollfd serverPollFd;
	serverPollFd.fd = _serverFd;
	serverPollFd.events = POLLIN;
	serverPollFd.revents = 0;
	_fds.push_back(serverPollFd);

	std::cout << "Server listening on port " << _port << std::endl;
}

Server::~Server()
{
	for (size_t i = 0; i < _clients.size(); i++)
		delete _clients[i];
	_clients.clear();
	
	for (size_t i = 0; i < _channels.size(); i++)
		delete _channels[i];
	_channels.clear();
	
	if (_serverFd >= 0)
		close(_serverFd);
}

Channel* Server::getChannel(const std::string& name)
{
    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (_channels[i]->getName() == name)
            return _channels[i];
    }
    return NULL;
}

std::string Server::getPassword() const
{
	return _password;
}

void Server::acceptNewClient()
{
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	
	int clientFd = accept(_serverFd, (struct sockaddr*)&clientAddr, &clientLen);
	if (clientFd < 0)
		return;

	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(clientFd);
		return;
	}

	Client* newClient = new Client(clientFd);
	_clients.push_back(newClient);

	struct pollfd clientPollFd;
	clientPollFd.fd = clientFd;
	clientPollFd.events = POLLIN;
	clientPollFd.revents = 0;
	_fds.push_back(clientPollFd);

	std::cout << "New client connected: " << clientFd << std::endl;
}

void Server::handleClientMessage(int index)
{
	char buffer[512];
	Client* client = _clients[index - 1];
	
	int bytesRead = recv(client->getFd(), buffer, sizeof(buffer) - 1, 0);
	
	if (bytesRead <= 0)
	{
		if (bytesRead == 0)
			std::cout << "Client " << client->getFd() << " disconnected" << std::endl;
		removeClient(index);
		return;
	}

	buffer[bytesRead] = '\0';
	client->appendBuffer(std::string(buffer));

	std::string buf = client->getBuffer();
	std::vector<std::string> messages = Parser::extractMessages(buf);
	client->clearBuffer();
	client->appendBuffer(buf);

	for (size_t i = 0; i < messages.size(); i++)
	{
		Command cmd = Parser::parseMessage(messages[i]);
		
		if (cmd.isValid())
			executeCommand(client, cmd);
	}
}

void Server::removeClient(int index)
{
	Client* client = _clients[index - 1];
	
	for (size_t i = 0; i < _channels.size(); ++i)
	{
		if (_channels[i]->isMember(client))
		{
			_channels[i]->removeMember(client);
			if (_channels[i]->isEmpty())
			{
				std::string channelName = _channels[i]->getName();
				removeChannel(channelName);
				--i;
			}
		}
	}
	
	close(client->getFd());
	delete client;
	_clients.erase(_clients.begin() + (index - 1));
	_fds.erase(_fds.begin() + index);
}

void Server::executeCommand(Client* client, const Command& cmd)
{
	std::string command = cmd.getCommand();
	
	std::cout << "Executing command: " << command << " from client " << client->getFd() << std::endl;
	
	if (command == "PASS")
		handlePass(client, cmd);
	else if (command == "NICK")
		handleNick(client, cmd);
	else if (command == "USER")
		handleUser(client, cmd);
	else if (command == "JOIN")
		handleJoin(client, cmd);
	else if (command == "PRIVMSG")
		handlePrivmsg(client, cmd);
	else if (command == "NOTICE")
		handleNotice(client, cmd);
	else
	{
		if (!client->isRegistered())
		{
			std::string reply = Utils::formatReply(ERR_NOTREGISTERED, "*", ":You have not registered");
			client->sendMessage(reply);
		}
	}
}

bool Server::isNicknameInUse(const std::string& nickname, Client* exclude)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i] != exclude && _clients[i]->getNickname() == nickname)
			return true;
	}
	return false;
}

void Server::run()
{
	while (true)
	{
		int pollCount = poll(&_fds[0], _fds.size(), -1);
		if (pollCount < 0)
		{
			std::cerr << "Poll error" << std::endl;
			break;
		}

		for (size_t i = 0; i < _fds.size(); i++)
		{
			if (_fds[i].revents & POLLIN)
			{
				if (_fds[i].fd == _serverFd)
					acceptNewClient();
				else
					handleClientMessage(i);
			}
		}
	}
}

Channel* Server::createChannel(const std::string& name)
{
	Channel* newChannel = new Channel(name);
	_channels.push_back(newChannel);
	std::cout << "Channel created: " << name << std::endl;
	return newChannel;
}

void Server::removeChannel(const std::string& name)
{
	for (size_t i = 0; i < _channels.size(); ++i)
	{
		if (_channels[i]->getName() == name)
		{
			delete _channels[i];
			_channels.erase(_channels.begin() + i);
			std::cout << "Channel removed: " << name << std::endl;
			return;
		}
	}
}

Client* Server::getClientByNickname(const std::string& nickname)
{
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i]->getNickname() == nickname)
			return _clients[i];
	}
	return NULL;
}
