/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
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
**	JOIN <channel>{,<channel>} [<key>{,<key>}]
**	- Not registered:                ERR_NOTREGISTERED-style reject
**	- No args:                       ERR_NEEDMOREPARAMS (461)
**	- Per channel name in the list:
**	    * Invalid name (no '#'/'&'): ERR_NOSUCHCHANNEL (403)
**	    * If channel does not exist: Channels::add_channel + make the
**	      joining client the first op (Channel::addOp).
**	    * If channel exists:
**	        - invite-only and not invited:  ERR_INVITEONLYCHAN (473)
**	        - key set and key mismatched:   ERR_BADCHANNELKEY (475)
**	        - user limit reached:           ERR_CHANNELISFULL (471)
**	    * Channel::addUser + Client::joinChannel
**	    * Broadcast ":<nick>!<user>@<host> JOIN <channel>" to all members.
**	    * Send RPL_TOPIC (332) or RPL_NOTOPIC (331), then RPL_NAMREPLY (353)
**	      and RPL_ENDOFNAMES (366) to the joining client.
*/
int	cmd_join(Command& cmd, Server& serv) {
	(void)cmd;
	(void)serv;
	return 0;
}

/*
**	PART <channel>{,<channel>} [:<reason>]
**	- No args:                       ERR_NEEDMOREPARAMS (461)
**	- Per channel:
**	    * Channel does not exist:    ERR_NOSUCHCHANNEL (403)
**	    * Client not in channel:     ERR_NOTONCHANNEL (442)
**	- Broadcast ":<nick>!<user>@<host> PART <chan> :<reason>" to all members
**	  (including the leaver), then Channel::removeUser / removeOp +
**	  Client::leaveChannel. If the channel becomes empty, drop it via
**	  Channels::remove_channel.
*/
int	cmd_part(Command& cmd, Server& serv) {
	(void)cmd;
	(void)serv;
	return 0;
}

/*
**	TOPIC <channel> [:<topic>]
**	- No channel arg:                ERR_NEEDMOREPARAMS (461)
**	- Channel does not exist:        ERR_NOSUCHCHANNEL (403)
**	- Client not in channel:         ERR_NOTONCHANNEL (442)
**	- Query mode (no trailing): respond with RPL_TOPIC (332) or
**	  RPL_NOTOPIC (331) to the requester only.
**	- Set mode (trailing present):
**	    * If +t mode is set (topic restricted) and client is not an op:
**	      ERR_CHANOPRIVSNEEDED (482).
**	    * Empty trailing => Channel::clearTopic; else Channel::setTopic.
**	    * Broadcast ":<nick>!<user>@<host> TOPIC <chan> :<topic>" to all
**	      members.
*/
int	cmd_topic(Command& cmd, Server& serv) {
	(void)cmd;
	(void)serv;
	return 0;
}

/*
**	KICK <channel> <user>{,<user>} [:<comment>]
**	- Missing args:                  ERR_NEEDMOREPARAMS (461)
**	- Channel does not exist:        ERR_NOSUCHCHANNEL (403)
**	- Requester not in channel:      ERR_NOTONCHANNEL (442)
**	- Requester not an op:           ERR_CHANOPRIVSNEEDED (482)
**	- Per target:
**	    * Target not in channel:     ERR_USERNOTINCHANNEL (441)
**	    * Broadcast ":<kicker>!<user>@<host> KICK <chan> <target> :<reason>"
**	      to all members (including the target).
**	    * Channel::removeUser / removeOp + Client::leaveChannel.
**	- Drop the channel if it becomes empty.
*/
int	cmd_kick(Command& cmd, Server& serv) {
	(void)cmd;
	(void)serv;
	return 0;
}

/*
**	INVITE <nickname> <channel>
**	- Missing args:                  ERR_NEEDMOREPARAMS (461)
**	- Target nick does not exist:    ERR_NOSUCHNICK (401)
**	- Channel does not exist:        ERR_NOSUCHCHANNEL (403)
**	- Requester not in channel:      ERR_NOTONCHANNEL (442)
**	- If channel is +i and requester is not an op:
**	                                 ERR_CHANOPRIVSNEEDED (482)
**	- Target already in channel:     ERR_USERONCHANNEL (443)
**	- On success:
**	    * record invitation (extra Channel state may be needed: an
**	      invite-list set<string>) so a subsequent JOIN bypasses +i.
**	    * Send RPL_INVITING (341) to requester.
**	    * Send ":<requester>!<user>@<host> INVITE <target> :<channel>"
**	      to the target.
*/
int	cmd_invite(Command& cmd, Server& serv) {
	(void)cmd;
	(void)serv;
	return 0;
}

/*
**	MODE <target> [<modestring> [<modeargs>...]]
**	- Channel target only (MODE on a nickname is out of scope for ft_irc).
**	- No channel:                    ERR_NEEDMOREPARAMS (461)
**	- Channel does not exist:        ERR_NOSUCHCHANNEL (403)
**	- Query mode (no modestring): respond with RPL_CHANNELMODEIS (324).
**	- Set/unset mode (modestring present):
**	    * Requester not an op:       ERR_CHANOPRIVSNEEDED (482)
**	    * Walk the modestring char by char, tracking +/-:
**	        i  -> Channel::setInviteOnly(true/false)
**	        t  -> topic-restricted flag (add as Channel state if missing)
**	        k  -> Channel::setKey(arg) / Channel::clearKey()
**	        o  -> Channel::addOp(arg) / Channel::removeOp(arg)
**	        l  -> Channel::setUserLimit(atoi(arg)) / setUserLimit(0)
**	        anything else:           ERR_UNKNOWNMODE (472)
**	    * Broadcast ":<nick>!<user>@<host> MODE <chan> <modestring> <args>"
**	      to all members.
*/
int	cmd_mode(Command& cmd, Server& serv) {
	(void)cmd;
	(void)serv;
	return 0;
}
