/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   misc.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 00:00:00 by johartma          #+#    #+#             */
/*   Updated: 2026/06/09 00:00:00 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command/Handlers.hpp"
#include "Server.hpp"

/*
**	PING <token>
**	- No token:                      ERR_NOORIGIN (409)
**	- On success: reply "PONG <serverName> :<token>" to the requester via
**	  serv.sndmsg. Does NOT require the client to be registered.
*/
int	cmd_ping(Command& cmd, Server& serv) {
	(void)cmd;
	(void)serv;
	return 0;
}

/*
**	PONG <token>
**	- Response to a server-initiated PING (keep-alive). For ft_irc this is
**	  effectively a no-op: just consume it. If/when server-side PING timers
**	  are added, this is where you clear the "awaiting PONG" flag on the
**	  client.
*/
int	cmd_pong(Command& cmd, Server& serv) {
	(void)cmd;
	(void)serv;
	return 0;
}

/*
**	cmd_unknown — fallback for CmdType::UNKNOWN.
**	- Send ERR_UNKNOWNCOMMAND (421):
**	    ":<serverName> 421 <nick> <commandToken> :Unknown command"
**	  Note: the original command token is no longer in `cmd` after parsing.
**	  Either stash the raw first token in Command during parsing, or just
**	  send a generic "*" placeholder for now.
*/
int	cmd_unknown(Command& cmd, Server& serv) {
	(void)cmd;
	(void)serv;
	return 0;
}
