/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/08 09:33:30 by johartma          #+#    #+#             */
/*   Updated: 2026/06/15 18:16:22 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command/Command.hpp"
#include "Command/Handlers.hpp"
#include "Server.hpp"
#include <string>
#include <sstream>
#include <map>


static void remove_end(std::string& raw) {
	if (!raw.empty() && raw[raw.size() - 1] == '\n')
		raw.erase(raw.size() - 1);
	if (!raw.empty() && raw[raw.size() - 1] == '\r')
		raw.erase(raw.size() - 1);
}

static std::string get_prefix(std::string& raw) {
	size_t		space;
	std::string	prefix;

	if (raw.empty())
		return "";
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

static std::string get_trailing(std::string& raw, bool& has) {
    size_t pos = raw.find(" :");
    if (pos == std::string::npos) {
        has = false;
        return "";
    }
    has = true;
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
	_has_trailing = false;
	_trailing = get_trailing(line, _has_trailing);
	
	std::istringstream	ss(line);
	ss >> token;
	_raw_cmd = token;
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

const std::string&	Command::getRawCmd() const
{
	return _raw_cmd;
}

const std::string&	Command::getTrailing() const
{
	return _trailing;
}

bool	Command::hasTrailing() const
{
	return _has_trailing;
}

typedef int (*Handler)(Command&, Server&);

static std::map<CmdType::Type, Handler>&	get_handlers()
{
	static std::map<CmdType::Type, Handler>	handlers;

	if (handlers.empty())
	{
		handlers[CmdType::PASS]    = &cmd_pass;
		handlers[CmdType::NICK]    = &cmd_nick;
		handlers[CmdType::USER]    = &cmd_user;
		handlers[CmdType::QUIT]    = &cmd_quit;
		handlers[CmdType::JOIN]    = &cmd_join;
		handlers[CmdType::PART]    = &cmd_part;
		handlers[CmdType::TOPIC]   = &cmd_topic;
		handlers[CmdType::KICK]    = &cmd_kick;
		handlers[CmdType::INVITE]  = &cmd_invite;
		handlers[CmdType::MODE]    = &cmd_mode;
		handlers[CmdType::PRIVMSG] = &cmd_privmsg;
		handlers[CmdType::NOTICE]  = &cmd_notice;
		handlers[CmdType::PING]    = &cmd_ping;
		handlers[CmdType::PONG]    = &cmd_pong;
	}
	return handlers;
}

int	Command::execute(Server& serv)
{
	std::map<CmdType::Type, Handler>&			handlers = get_handlers();
	std::map<CmdType::Type, Handler>::iterator	it = handlers.find(_type);

	if (it == handlers.end())
		return cmd_unknown(*this, serv);
	return it->second(*this, serv);
}
