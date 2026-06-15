/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   messaging.cpp                                      :+:      :+:    :+:   */
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
**	PRIVMSG <target>{,<target>} :<text>
**	- No target:                     ERR_NORECIPIENT (411)
**	- No trailing text:              ERR_NOTEXTTOSEND (412)
**	- Per target:
**	    * Target starts with '#' or '&' => channel:
**	        - Channel does not exist:                ERR_NOSUCHNICK (401)
**	          (per RFC the same numeric is reused for channel-or-nick)
**	        - Sender not in channel and channel is +n (no external msgs):
**	                                                 ERR_CANNOTSENDTOCHAN (404)
**	        - Forward ":<sender>!<u>@<h> PRIVMSG <chan> :<text>" to every
**	          channel member EXCEPT the sender.
**	    * Otherwise treat as nickname:
**	        - Lookup via serv.getClients().get_client(target).
**	        - Not found:                              ERR_NOSUCHNICK (401)
**	        - Forward ":<sender>!<u>@<h> PRIVMSG <target> :<text>" to the
**	          target client via serv.sndmsg.
*/
int	cmd_privmsg(Command& cmd, Server& serv) {
	(void)cmd;
	(void)serv;
	return 0;
}

/*
**	NOTICE <target>{,<target>} :<text>
**	- Same dispatch logic as PRIVMSG, BUT: per RFC, a NOTICE must NEVER
**	  generate an automatic reply (no numeric error replies, no ERROR).
**	- Silently drop on any failure (unknown target, channel missing, +n
**	  violation, etc.). Just forward on success.
*/
int	cmd_notice(Command& cmd, Server& serv) {
	(void)cmd;
	(void)serv;
	return 0;
}
