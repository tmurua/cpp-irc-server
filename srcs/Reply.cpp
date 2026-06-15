/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reply.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 00:00:00 by johartma          #+#    #+#             */
/*   Updated: 2026/06/09 00:00:00 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Reply.hpp"

namespace Reply {

// ── Primitives ────────────────────────────────────────────────────────────────

std::string from_server(const std::string& server, const std::string& code,
                        const std::string& params, const std::string& trailing)
{
	std::string msg = ":" + server + " " + code + " " + params;
	if (!trailing.empty())
		msg += " :" + trailing;
	msg += "\r\n";
	return msg;
}

std::string from_client(const Client& client, const std::string& cmd,
                        const std::string& params, const std::string& trailing)
{
	std::string msg = ":" + client.getNickname() + "!" + client.getUsername()
	                + "@" + client.getHostname() + " " + cmd;
	if (!params.empty())
		msg += " " + params;
	if (!trailing.empty())
		msg += " :" + trailing;
	msg += "\r\n";
	return msg;
}

// ── Registration numerics ─────────────────────────────────────────────────────

std::string rpl_welcome(const std::string& server, const Client& client)
{
	(void)server;
	(void)client;
	return "";
}

std::string rpl_yourhost(const std::string& server, const Client& client)
{
	(void)server;
	(void)client;
	return "";
}

std::string rpl_created(const std::string& server, const Client& client)
{
	(void)server;
	(void)client;
	return "";
}

std::string rpl_myinfo(const std::string& server, const Client& client)
{
	(void)server;
	(void)client;
	return "";
}

// ── Channel numerics ──────────────────────────────────────────────────────────

std::string rpl_notopic(const std::string& server, const std::string& nick,
                        const std::string& channel)
{
	(void)server;
	(void)nick;
	(void)channel;
	return "";
}

std::string rpl_topic(const std::string& server, const std::string& nick,
                      const std::string& channel, const std::string& topic)
{
	(void)server;
	(void)nick;
	(void)channel;
	(void)topic;
	return "";
}

std::string rpl_inviting(const std::string& server, const std::string& nick,
                         const std::string& invited, const std::string& channel)
{
	(void)server;
	(void)nick;
	(void)invited;
	(void)channel;
	return "";
}

std::string rpl_namreply(const std::string& server, const std::string& nick,
                         const std::string& channel, const std::string& names)
{
	(void)server;
	(void)nick;
	(void)channel;
	(void)names;
	return "";
}

std::string rpl_endofnames(const std::string& server, const std::string& nick,
                           const std::string& channel)
{
	(void)server;
	(void)nick;
	(void)channel;
	return "";
}

// ── Error numerics ────────────────────────────────────────────────────────────

std::string err_nosuchnick(const std::string& server, const std::string& nick,
                           const std::string& target)
{
	(void)server;
	(void)nick;
	(void)target;
	return "";
}

std::string err_nosuchchannel(const std::string& server, const std::string& nick,
                              const std::string& channel)
{
	(void)server;
	(void)nick;
	(void)channel;
	return "";
}

std::string err_norecipient(const std::string& server, const std::string& nick,
                            const std::string& cmd)
{
	(void)server;
	(void)nick;
	(void)cmd;
	return "";
}

std::string err_notexttosend(const std::string& server, const std::string& nick)
{
	(void)server;
	(void)nick;
	return "";
}

std::string err_noorigin(const std::string& server, const std::string& nick)
{
	(void)server;
	(void)nick;
	return "";
}

std::string err_unknowncommand(const std::string& server, const std::string& nick,
                               const std::string& cmd)
{
	(void)server;
	(void)nick;
	(void)cmd;
	return "";
}

std::string err_nonicknamegiven(const std::string& server, const std::string& nick)
{
	(void)server;
	(void)nick;
	return "";
}

std::string err_erroneusnickname(const std::string& server, const std::string& nick,
                                 const std::string& bad_nick)
{
	(void)server;
	(void)nick;
	(void)bad_nick;
	return "";
}

std::string err_nicknameinuse(const std::string& server, const std::string& nick,
                              const std::string& taken_nick)
{
	(void)server;
	(void)nick;
	(void)taken_nick;
	return "";
}

std::string err_notonchannel(const std::string& server, const std::string& nick,
                             const std::string& channel)
{
	(void)server;
	(void)nick;
	(void)channel;
	return "";
}

std::string err_useronchannel(const std::string& server, const std::string& nick,
                              const std::string& target, const std::string& channel)
{
	(void)server;
	(void)nick;
	(void)target;
	(void)channel;
	return "";
}

std::string err_needmoreparams(const std::string& server, const std::string& nick,
                               const std::string& cmd)
{
	(void)server;
	(void)nick;
	(void)cmd;
	return "";
}

std::string err_alreadyregistred(const std::string& server, const std::string& nick)
{
	(void)server;
	(void)nick;
	return "";
}

std::string err_passwdmismatch(const std::string& server, const std::string& nick)
{
	(void)server;
	(void)nick;
	return "";
}

std::string err_channelisfull(const std::string& server, const std::string& nick,
                              const std::string& channel)
{
	(void)server;
	(void)nick;
	(void)channel;
	return "";
}

std::string err_inviteonlychan(const std::string& server, const std::string& nick,
                               const std::string& channel)
{
	(void)server;
	(void)nick;
	(void)channel;
	return "";
}

std::string err_badchannelkey(const std::string& server, const std::string& nick,
                              const std::string& channel)
{
	(void)server;
	(void)nick;
	(void)channel;
	return "";
}

std::string err_chanoprivsneeded(const std::string& server, const std::string& nick,
                                 const std::string& channel)
{
	(void)server;
	(void)nick;
	(void)channel;
	return "";
}

} // namespace Reply
