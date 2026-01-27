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

std::vector<std::string> Utils::splitByComma(const std::string& str)
{
    std::vector<std::string> result;
    std::string current;
    
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (str[i] == ',')
        {
            if (!current.empty())
            {
                result.push_back(current);
                current.clear();
            }
        }
        else
        {
            current += str[i];
        }
    }
    
    if (!current.empty())
        result.push_back(current);
    
    return result;
}

bool Utils::isChannelName(const std::string& name)
{
    return !name.empty() && (name[0] == '#' || name[0] == '&');
}

bool Utils::isValidChannelName(const std::string& name)
{
    if (name.empty() || (name[0] != '#' && name[0] != '&'))
        return false;
    
    for (size_t i = 0; i < name.length(); ++i)
    {
        char c = name[i];
        if (c == ' ' || c == ',' || c == '\07' || c < 33)
            return false;
    }
    
    return true;
}