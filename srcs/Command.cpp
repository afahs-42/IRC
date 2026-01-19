/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 21:09:00 by marvin            #+#    #+#             */
/*   Updated: 2026/01/19 21:09:00 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

Command::Command() : _prefix(""), _command(""), _trailing(""), _valid(true) {}

Command::Command(const Command& src)
{
    *this = src;
}

Command& Command::operator=(const Command& other) 
{
    if (this != &other) 
    {
        _prefix = other._prefix;
        _command = other._command;
        _params = other._params;
        _trailing = other._trailing;
        _valid = other._valid;
    }
    return *this;
}

Command::~Command() {}

const std::string& Command::getPrefix() const
{
    return _prefix;
}

const std::string& Command::getCommand() const
{
    return _command;
}

const std::vector<std::string>& Command::getParams() const
{
    return _params;
}

const std::string& Command::getTrailing() const
{
    return _trailing;
}

bool Command::isValid() const
{
    return _valid;
}

void Command::setPrefix(const std::string& prefix)
{
    _prefix = prefix;
}

void Command::setCommand(const std::string& command)
{
    _command = command;
}

void Command::addParam(const std::string& param)
{
    _params.push_back(param);
}

void Command::setTrailing(const std::string& trailing)
{
    _trailing = trailing;
}

void Command::setValid(bool valid)
{
    _valid = valid;
}

bool Command::isValidCommand(const std::string& cmd)
{
    const std::string validCommands[] = {
        "NICK", "USER", "PASS", "JOIN", "PART", "PRIVMSG",
        "PING", "PONG", "KICK", "MODE","TOPIC", "INVITE"};
    
    size_t count = sizeof(validCommands) / sizeof(validCommands[0]);
    for (size_t i = 0; i < count; i++)
    {
        if (cmd == validCommands[i])
            return true;
    }
    return false;
}