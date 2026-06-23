/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handlers.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 00:00:00 by johartma          #+#    #+#             */
/*   Updated: 2026/06/15 18:14:06 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HANDLERS_HPP
# define HANDLERS_HPP

# include "Command/Command.hpp"

class Server;

/*
**	IRC command handlers.
**
**	Every handler has the same signature:
**		int cmd_xxx(Command& cmd, Server& serv);
**
**	Return value: 0 on success, non-zero on error (matches the convention
**	used by Clients/Channels managers).
**
**	Handlers read the parsed message via `cmd` (getPrefix / getType /
**	getArguments / getTrailing / getFd) and mutate server state through
**	`serv.getClients()` / `serv.getChannels()`. Wire output is sent
**	exclusively via `serv.sndmsg(client, msg)`.
**
**	Handlers are looked up by CmdType::Type in a static function table
**	inside Command::execute(); see srcs/Command/Command.cpp.
*/

// Auth / registration (srcs/Command/handlers/auth.cpp)
int	cmd_pass(Command& cmd, Server& serv);
int	cmd_nick(Command& cmd, Server& serv);
int	cmd_user(Command& cmd, Server& serv);
int	cmd_quit(Command& cmd, Server& serv);

// Channel operations (srcs/Command/handlers/channel.cpp)
int	cmd_join(Command& cmd, Server& serv);
int	cmd_part(Command& cmd, Server& serv);
int	cmd_topic(Command& cmd, Server& serv);
int	cmd_kick(Command& cmd, Server& serv);
int	cmd_invite(Command& cmd, Server& serv);
int	cmd_mode(Command& cmd, Server& serv);

// Messaging (srcs/Command/handlers/messaging.cpp)
int	cmd_privmsg(Command& cmd, Server& serv);
int	cmd_notice(Command& cmd, Server& serv);

// Misc / connection (srcs/Command/handlers/misc.cpp)
int	cmd_ping(Command& cmd, Server& serv);
int	cmd_pong(Command& cmd, Server& serv);
int	cmd_cap(Command& cmd, Server& serv);
int	cmd_who(Command& cmd, Server& serv);
int	cmd_unknown(Command& cmd, Server& serv);

#endif
