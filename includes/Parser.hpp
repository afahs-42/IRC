/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/17 22:20:49 by marvin            #+#    #+#             */
/*   Updated: 2026/01/17 22:20:49 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <iostream>
#include "Command.hpp"

class Parser {
    private:
        Parser();
        Parser(const Parser& src);
        Parser& operator=(const Parser& rhs);
        ~Parser();
    public:
        static Command parseMessage(const std::string& raw);
        static bool isComplete(const std::string& buffer);
        static std::vector<std::string> extractMessages(std::string& buffer);
};

#endif