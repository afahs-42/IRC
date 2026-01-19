/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/17 23:50:34 by marvin            #+#    #+#             */
/*   Updated: 2026/01/17 23:50:34 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include <stdexcept>

Parser::Parser() {}

Parser::Parser(const Parser& src) {
    (void)src;
}

Parser& Parser::operator=(const Parser& other) {
    (void)other;
    return *this;
}

Parser::~Parser() {}

Command Parser::parseMessage(const std::string& raw)
{
    Command cmd;
    std::string line = raw;
    size_t pos = 0;

    if (raw.empty())
    {
        cmd.setValid(false);
        return cmd;
    }
    if (line.length() > 512)
    {
        std::cerr << "Message exceeds 512 bytes limit" << std::endl;
        cmd.setValid(false);
        return cmd;
    }
    if (line.size() >= 2 && line.substr(line.size() - 2) == "\r\n")
        line.erase(line.size() - 2);
    if (raw.find('\n') != std::string::npos &&
        raw.find("\r\n") == std::string::npos)
    {
        std::cerr << "Malformed line ending" << std::endl;
        cmd.setValid(false);
        return cmd;
    }
    if (!line.empty() && line[0] == ':')
    {
        size_t space = line.find(' ');
        if (space == std::string::npos)
        {
            std::cerr << "Malformed prefix" << std::endl;
            cmd.setValid(false);
            return cmd;
        }
        cmd.setPrefix(line.substr(1, space - 1));
        pos = space + 1;
    }
    size_t space = line.find(' ', pos);
    if (space == std::string::npos)
        cmd.setCommand(line.substr(pos));
    else
    {
        cmd.setCommand(line.substr(pos, space - pos));
        pos = space + 1;
    }
    if (cmd.getCommand().empty())
    {
        std::cerr << "Empty command" << std::endl;
        cmd.setValid(false);
        return cmd;
    }
    if (!Command::isValidCommand(cmd.getCommand()))
    {
        std::cerr << "Unknown command: " << cmd.getCommand() << std::endl;
        cmd.setValid(false);
        return cmd;
    }
    while (pos < line.size())
    {
        if (line[pos] == ':')
        {
            cmd.setTrailing(line.substr(pos + 1));
            break;
        }
        size_t nextSpace = line.find(' ', pos);
        if (nextSpace == std::string::npos)
        {
            cmd.addParam(line.substr(pos));
            break;
        }
        cmd.addParam(line.substr(pos, nextSpace - pos));

        pos = nextSpace + 1;
    }
    return cmd;
}

bool Parser::isComplete(const std::string& buffer)
{
    return buffer.find("\r\n") != std::string::npos;
}

std::vector<std::string> Parser::extractMessages(std::string& buffer)
{
    std::vector<std::string> messages;
    size_t pos;

    if (buffer.empty())
        return messages;

    while ((pos = buffer.find("\r\n")) != std::string::npos) 
    {
        if (pos == 0)
        {
            buffer.erase(0, 2);
            continue;
        }
        std::string line = buffer.substr(0, pos + 2);
        messages.push_back(line);
        buffer.erase(0, pos + 2);
    }
    return messages;
}

