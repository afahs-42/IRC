#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
private:
	int _fd;
	std::string _nickname;
	std::string _username;
	std::string _buffer;
	bool _authenticated;
	bool _registered; 
    bool _hasPassword; 

	Client();

public:
	Client(int fd);
	~Client();

	int getFd() const;
	std::string getNickname() const;
	std::string getUsername() const;
	std::string getBuffer() const;
	bool isAuthenticated() const;

	void setNickname(const std::string& nickname);
	void setUsername(const std::string& username);
	void appendBuffer(const std::string& data);
	void clearBuffer();
	void setAuthenticated(bool auth);

	void sendMessage(const std::string& message);

	bool isRegistered() const;
    bool hasPassword() const;
    void setRegistered(bool registered);
    void setHasPassword(bool hasPass);
};

#endif
