#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

#define RPL_WELCOME 001
#define RPL_YOURHOST 002
#define RPL_CREATED 003
#define RPL_MYINFO 004

#define ERR_NOSUCHNICK 401
#define ERR_NOSUCHCHANNEL 403
#define ERR_CANNOTSENDTOCHAN 404
#define ERR_TOOMANYCHANNELS 405
#define ERR_TOOMANYTARGETS 407
#define ERR_NOTONCHANNEL 442
#define ERR_CHANNELISFULL 471
#define ERR_INVITEONLYCHAN 473
#define ERR_BADCHANNELKEY 475
#define ERR_BADCHANMASK 476
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
    static std::vector<std::string> splitByComma(const std::string& str);
    static bool isChannelName(const std::string& name);
    static bool isValidChannelName(const std::string& name);
};

#endif