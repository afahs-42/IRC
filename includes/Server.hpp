#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <poll.h>
#include "Client.hpp"
#include "Command.hpp"

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
	
	void executeCommand(Client* client, const Command& cmd);
	void handlePass(Client* client, const Command& cmd);
	void handleNick(Client* client, const Command& cmd);
	void handleUser(Client* client, const Command& cmd);
	
	bool isNicknameInUse(const std::string& nickname, Client* exclude);
	void sendWelcome(Client* client);

public:
	Server(int port, const std::string& password);
	~Server();

	void run();
	std::string getPassword() const;
};

#endif
