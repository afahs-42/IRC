/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/17 22:28:21 by marvin            #+#    #+#             */
/*   Updated: 2026/01/19 15:30:00 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <vector>

class Command
{
    private:
        std::string _prefix;
        std::string _command;
        std::vector<std::string> _params;
        std::string _trailing;
        bool _valid;

    public:
        Command();
        Command(const Command& src);
        Command& operator=(const Command& rhs);
        ~Command();

        const std::string& getPrefix() const;
        const std::string& getCommand() const;
        const std::vector<std::string>& getParams() const;
        const std::string& getTrailing() const;
        bool isValid() const;

        void setPrefix(const std::string& prefix);
        void setCommand(const std::string& command);
        void addParam(const std::string& param);
        void setTrailing(const std::string& trailing);
        void setValid(bool valid);

        static bool isValidCommand(const std::string& cmd);
};

#endif