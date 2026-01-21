#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <poll.h>
#include "Client.hpp"

class Server
{
private:
	int _serverFd;
	int _port;
	std::string _password;
	std::vector<Client*> _clients;
	std::vector<struct pollfd> _fds;

	Server();
	Server(const Server& other);
	Server& operator=(const Server& other);

	void acceptNewClient();
	void handleClientMessage(int index);
	void removeClient(int index);

public:
	Server(int port, const std::string& password);
	~Server();

	void run();
};

#endif
