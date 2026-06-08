/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/08 09:33:30 by johartma          #+#    #+#             */
/*   Updated: 2026/06/08 10:44:17 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command/Command.hpp"
#include "Server.hpp"
#include <string>
#include <sstream>


static void remove_end(std::string& raw) {
	if (!raw.empty() && raw[raw.size() - 1] == '\n')
		raw.erase(raw.size() - 1);
	if (!raw.empty() && raw[raw.size() - 1] == '\r')
		raw.erase(raw.size() - 1);
}

static std::string get_prefix(std::string& raw) {
	size_t		space;
	std::string	prefix;
	
	if (raw[0] == ':') {
		space = raw.find(' ');
		if (space == std::string::npos)
			throw Command::MalformedException();
		prefix = raw.substr(1, space - 1);
		raw = raw.substr(space + 1);
	}
	else
		prefix = "";
	return prefix;
}

static std::string get_trailing(std::string& raw) {
    size_t pos = raw.find(" :");
    if (pos == std::string::npos)
        return "";
    std::string trailing = raw.substr(pos + 2); // can be empty, that's okay
    raw = raw.substr(0, pos);
    return trailing;
}

Command::Command(int fd, const std::string& raw)
{
	std::string line = raw;
	std::string token;
	_fd = fd;
	
	remove_end(line);
	_prefix = get_prefix(line);
	_trailing = get_trailing(line);
	
	std::istringstream	ss(line);
	ss >> token;
	_type = parse_cmdtype(token);
	while (ss >> token)
		_arguments.push_back(token);
}

Command::~Command()
{
	
}

int	Command::getFd() const
{
	return _fd;
}

const std::string&	Command::getPrefix() const
{
	return _prefix;
}

CmdType::Type	Command::getType() const
{
	return _type;
}

const std::vector<std::string>&	Command::getArguments() const
{
	return _arguments;
}

const std::string&	Command::getTrailing() const
{
	return _trailing;
}

int	Command::execute(Server& serv)
{
	(void)serv;
	return 0;
}
