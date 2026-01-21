#include "Server.hpp"
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
	
	if (_serverFd >= 0)
		close(_serverFd);
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
	size_t pos;
	while ((pos = buf.find("\r\n")) != std::string::npos)
	{
		std::string message = buf.substr(0, pos);
		buf.erase(0, pos + 2);
		
		std::cout << "Received from " << client->getFd() << ": " << message << std::endl;
	}
	
	client->clearBuffer();
	client->appendBuffer(buf);
}

void Server::removeClient(int index)
{
	Client* client = _clients[index - 1];
	close(client->getFd());
	
	delete client;
	_clients.erase(_clients.begin() + (index - 1));
	_fds.erase(_fds.begin() + index);
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
