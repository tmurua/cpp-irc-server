/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reply.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/15 18:16:30 by johartma          #+#    #+#             */
/*   Updated: 2026/06/15 18:16:33 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Reply.hpp"
#include <sstream>

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
	return from_server(server, "001", client.getNickname(),
		"Welcome to the IRC Network "
		+ client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname());
}

std::string rpl_yourhost(const std::string& server, const Client& client)
{
	return from_server(server, "002", client.getNickname(),
		"Your host is " + server + ", running version 1.0");
}

std::string rpl_created(const std::string& server, const Client& client)
{
	return from_server(server, "003", client.getNickname(),
		"This server was created sometime");
}

std::string rpl_myinfo(const std::string& server, const Client& client)
{
	return from_server(server, "004",
		client.getNickname() + " " + server + " 1.0 o itkol");
}

// ── Channel numerics ──────────────────────────────────────────────────────────

std::string rpl_notopic(const std::string& server, const std::string& nick,
                        const std::string& channel)
{
	return from_server(server, "331", nick + " " + channel, "No topic is set");
}

std::string rpl_topic(const std::string& server, const std::string& nick,
                      const std::string& channel, const std::string& topic)
{
	return from_server(server, "332", nick + " " + channel, topic);
}

std::string rpl_inviting(const std::string& server, const std::string& nick,
                         const std::string& invited, const std::string& channel)
{
	return from_server(server, "341", nick + " " + invited + " " + channel);
}

std::string rpl_namreply(const std::string& server, const std::string& nick,
                         const std::string& channel, const std::string& names)
{
	return from_server(server, "353", nick + " = " + channel, names);
}

std::string rpl_endofnames(const std::string& server, const std::string& nick,
                           const std::string& channel)
{
	return from_server(server, "366", nick + " " + channel, "End of NAMES list");
}

std::string rpl_whoreply(const std::string& server, const std::string& nick,
                         const Client& member, const std::string& channel,
                         const std::string& flags)
{
	std::string realname = member.getRealname().empty()
		? member.getNickname() : member.getRealname();
	return from_server(server, "352",
		nick + " " + channel + " " + member.getUsername() + " "
		+ member.getHostname() + " " + server + " " + member.getNickname()
		+ " " + flags,
		"0 " + realname);
}

std::string rpl_endofwho(const std::string& server, const std::string& nick,
                         const std::string& name)
{
	return from_server(server, "315", nick + " " + name, "End of WHO list");
}

std::string rpl_endofbanlist(const std::string& server, const std::string& nick,
                             const std::string& channel)
{
	return from_server(server, "368", nick + " " + channel,
		"End of Channel Ban List");
}

std::string rpl_channelmodeis(const std::string& server, const std::string& nick,
                              const std::string& channel, const std::string& modestring)
{
	return from_server(server, "324", nick + " " + channel + " " + modestring);
}

// ── Error numerics ────────────────────────────────────────────────────────────

std::string err_nosuchnick(const std::string& server, const std::string& nick,
                           const std::string& target)
{
	return from_server(server, "401", nick + " " + target, "No such nick/channel");
}

std::string err_nosuchchannel(const std::string& server, const std::string& nick,
                              const std::string& channel)
{
	return from_server(server, "403", nick + " " + channel, "No such channel");
}

std::string err_cannotsendtochan(const std::string& server, const std::string& nick,
                                 const std::string& channel)
{
	return from_server(server, "404", nick + " " + channel,
		"Cannot send to channel");
}

std::string err_norecipient(const std::string& server, const std::string& nick,
                            const std::string& cmd)
{
	return from_server(server, "411", nick, "No recipient given (" + cmd + ")");
}

std::string err_notexttosend(const std::string& server, const std::string& nick)
{
	return from_server(server, "412", nick, "No text to send");
}

std::string err_noorigin(const std::string& server, const std::string& nick)
{
	return from_server(server, "409", nick, "No origin specified");
}

std::string err_unknowncommand(const std::string& server, const std::string& nick,
                               const std::string& cmd)
{
	return from_server(server, "421", nick + " " + cmd, "Unknown command");
}

std::string err_nonicknamegiven(const std::string& server, const std::string& nick)
{
	return from_server(server, "431", nick, "No nickname given");
}

std::string err_erroneusnickname(const std::string& server, const std::string& nick,
                                 const std::string& bad_nick)
{
	return from_server(server, "432", nick + " " + bad_nick, "Erroneous nickname");
}

std::string err_nicknameinuse(const std::string& server, const std::string& nick,
                              const std::string& taken_nick)
{
	return from_server(server, "433", nick + " " + taken_nick,
		"Nickname is already in use");
}

std::string err_usernotinchannel(const std::string& server, const std::string& nick,
                                 const std::string& target, const std::string& channel)
{
	return from_server(server, "441", nick + " " + target + " " + channel,
		"They aren't on that channel");
}

std::string err_notonchannel(const std::string& server, const std::string& nick,
                             const std::string& channel)
{
	return from_server(server, "442", nick + " " + channel,
		"You're not on that channel");
}

std::string err_useronchannel(const std::string& server, const std::string& nick,
                              const std::string& target, const std::string& channel)
{
	return from_server(server, "443", nick + " " + target + " " + channel,
		"is already on channel");
}

std::string err_needmoreparams(const std::string& server, const std::string& nick,
                               const std::string& cmd)
{
	return from_server(server, "461", nick + " " + cmd, "Not enough parameters");
}

std::string err_alreadyregistred(const std::string& server, const std::string& nick)
{
	return from_server(server, "462", nick,
		"Unauthorized command (already registered)");
}

std::string err_notregistered(const std::string& server, const std::string& nick)
{
	const std::string& target = nick.empty() ? std::string("*") : nick;
	return from_server(server, "451", target, "You have not registered");
}

std::string err_passwdmismatch(const std::string& server, const std::string& nick)
{
	return from_server(server, "464", nick, "Password incorrect");
}

std::string err_channelisfull(const std::string& server, const std::string& nick,
                              const std::string& channel)
{
	return from_server(server, "471", nick + " " + channel,
		"Cannot join channel (+l)");
}

std::string err_inviteonlychan(const std::string& server, const std::string& nick,
                               const std::string& channel)
{
	return from_server(server, "473", nick + " " + channel,
		"Cannot join channel (+i)");
}

std::string err_badchannelkey(const std::string& server, const std::string& nick,
                              const std::string& channel)
{
	return from_server(server, "475", nick + " " + channel,
		"Cannot join channel (+k)");
}

std::string err_chanoprivsneeded(const std::string& server, const std::string& nick,
                                 const std::string& channel)
{
	return from_server(server, "482", nick + " " + channel,
		"You're not channel operator");
}

std::string err_usersdontmatch(const std::string& server, const std::string& nick)
{
	return from_server(server, "502", nick, "Cant change mode for other users");
}

std::string err_unknownmode(const std::string& server, const std::string& nick,
                            char modechar)
{
	return from_server(server, "472",
		nick + " " + std::string(1, modechar), "is unknown mode char to me");
}

} // namespace Reply
