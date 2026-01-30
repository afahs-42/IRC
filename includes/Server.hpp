#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <poll.h>
#include "Client.hpp"
#include "Command.hpp"
#include "Channel.hpp"

class Server
{
private:
	int _serverFd;
	int _port;
	std::string _password;
	std::vector<Client*> _clients;
	std::vector<struct pollfd> _fds;
	std::vector<Channel*> _channels;

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
	void handleJoin(Client* client, const Command& cmd);
	void handlePrivmsg(Client* client, const Command& cmd);
	void handlePartAll(Client* client);
	void handleChannelMessage(Client* client, const std::string& channelName, const std::string& message);
	void handlePrivateMessage(Client* sender, const std::string& targetNick, const std::string& message);
	void handleNotice(Client* client, const Command& cmd);
	
	bool isNicknameInUse(const std::string& nickname, Client* exclude);
	void sendWelcome(Client* client);

public:
	Server(int port, const std::string& password);
	~Server();

	void run();
	std::string getPassword() const;
	
	Channel* createChannel(const std::string& name);
	void removeChannel(const std::string& name);
	Channel* getChannel(const std::string& name);
	Client* getClientByNickname(const std::string& nickname);
};

#endif