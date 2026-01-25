#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

#define RPL_WELCOME 001
#define RPL_YOURHOST 002
#define RPL_CREATED 003
#define RPL_MYINFO 004

#define ERR_NEEDMOREPARAMS 461
#define ERR_ALREADYREGISTRED 462
#define ERR_PASSWDMISMATCH 464
#define ERR_NONICKNAMEGIVEN 431
#define ERR_ERRONEUSNICKNAME 432
#define ERR_NICKNAMEINUSE 433
#define ERR_NOTREGISTERED 451

class Utils
{
public:
	static std::string formatReply(int code, const std::string& client, const std::string& message);
	static std::string formatMessage(const std::string& prefix, const std::string& command, const std::string& params);
	static std::string intToString(int num);
};

#endif