#include "Utils.hpp"
#include <sstream>

std::string Utils::intToString(int num)
{
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

std::string Utils::formatReply(int code, const std::string& client, const std::string& message)
{
	std::string codeStr;
	if (code < 10)
		codeStr = "00" + intToString(code);
	else if (code < 100)
		codeStr = "0" + intToString(code);
	else
		codeStr = intToString(code);
	
	return ":server " + codeStr + " " + client + " " + message;
}

std::string Utils::formatMessage(const std::string& prefix, const std::string& command, const std::string& params)
{
	std::string msg;
	if (!prefix.empty())
		msg = ":" + prefix + " ";
	msg += command + " " + params;
	return msg;
}
