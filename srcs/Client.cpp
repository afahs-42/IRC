#include "Client.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <cerrno>
#include <iostream>

Client::Client(int fd) : _fd(fd), _authenticated(false), _registered(false), _hasPassword(false)
{
}

Client::~Client()
{
}

int Client::getFd() const
{
	return _fd;
}

std::string Client::getNickname() const
{
	return _nickname;
}

std::string Client::getUsername() const
{
	return _username;
}

std::string Client::getBuffer() const
{
	return _buffer;
}

bool Client::isAuthenticated() const
{
	return _authenticated;
}

bool Client::isRegistered() const
{
	return _registered;
}

bool Client::hasPassword() const
{
	return _hasPassword;
}

void Client::setNickname(const std::string& nickname)
{
	_nickname = nickname;
}

void Client::setUsername(const std::string& username)
{
	_username = username;
}

void Client::appendBuffer(const std::string& data)
{
	_buffer += data;
}

void Client::clearBuffer()
{
	_buffer.clear();
}

void Client::setAuthenticated(bool auth)
{
	_authenticated = auth;
}

void Client::setRegistered(bool registered)
{
	_registered = registered;
}

void Client::setHasPassword(bool hasPass)
{
	_hasPassword = hasPass;
}

void Client::sendMessage(const std::string& message)
{
	std::string msg = message + "\r\n";
	size_t totalSent = 0;
	size_t msgLen = msg.length();

	while (totalSent < msgLen)
	{
		ssize_t sent = send(_fd, msg.c_str() + totalSent, msgLen - totalSent, 0);
		
		if (sent < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				continue;
			}
			else if (errno == EPIPE || errno == ECONNRESET)
			{
				std::cerr << "Client " << _fd << " connection broken" << std::endl;
				return;
			}
			else
			{
				std::cerr << "Send error for client " << _fd << std::endl;
				return;
			}
		}
		totalSent += sent;
	}
}
