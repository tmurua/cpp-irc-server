/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Reply.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/15 16:33:19 by johartma          #+#    #+#             */
/*   Updated: 2026/06/15 18:14:11 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef REPLY_HPP
# define REPLY_HPP

# include <string>
# include "Clients/Client.hpp"

/*
**	Reply — IRC wire-format builders.
**
**	Two primitives cover every shape a message can take; named functions are
**	thin wrappers that fill in the numeric code and fixed text so handlers
**	stay readable and formatting stays in one place.
**
**	All returned strings are terminated with \r\n.
*/

namespace Reply {

	// ── Primitives ────────────────────────────────────────────────────────────

	// Builds:  :<server> <code> <params> [:<trailing>]\r\n
	// Use for every server-originated numeric or command reply.
	// `params` holds the target nick and any middle parameters, space-separated.
	// `trailing` is optional; when non-empty it is prefixed with a colon.
	std::string from_server(const std::string& server, const std::string& code,
	                        const std::string& params, const std::string& trailing = "");

	// Builds:  :<nick>!<user>@<host> <cmd> [<params>] [:<trailing>]\r\n
	// Use when relaying a client action to other clients (JOIN, PART, NICK,
	// PRIVMSG, NOTICE, KICK, QUIT, TOPIC, INVITE, ...).
	// `params` and `trailing` are both optional.
	std::string from_client(const Client& client, const std::string& cmd,
	                        const std::string& params = "", const std::string& trailing = "");

	// ── Registration numerics ─────────────────────────────────────────────────

	// RPL_WELCOME (001) — send when PASS + NICK + USER all succeed and the
	//   client transitions to the registered state.
	std::string rpl_welcome(const std::string& server, const Client& client);

	// RPL_YOURHOST (002) — send immediately after RPL_WELCOME.
	std::string rpl_yourhost(const std::string& server, const Client& client);

	// RPL_CREATED (003) — send immediately after RPL_YOURHOST.
	std::string rpl_created(const std::string& server, const Client& client);

	// RPL_MYINFO (004) — send immediately after RPL_CREATED.
	//   Completes the welcome burst; client considers itself online after this.
	std::string rpl_myinfo(const std::string& server, const Client& client);

	// ── Channel numerics ──────────────────────────────────────────────────────

	// RPL_NOTOPIC (331) — reply to TOPIC when no topic is currently set.
	std::string rpl_notopic(const std::string& server, const std::string& nick,
	                        const std::string& channel);

	// RPL_TOPIC (332) — reply to TOPIC when a topic is set; also sent to a
	//   joining client (after the JOIN relay) when the channel has a topic.
	std::string rpl_topic(const std::string& server, const std::string& nick,
	                      const std::string& channel, const std::string& topic);

	// RPL_INVITING (341) — confirmation sent to the *inviting* client after
	//   INVITE succeeds (before the INVITE relay to the target).
	std::string rpl_inviting(const std::string& server, const std::string& nick,
	                         const std::string& invited, const std::string& channel);

	// RPL_NAMREPLY (353) — one line of nick names, sent during a NAMES reply
	//   and at the end of a JOIN sequence. May be repeated for large channels.
	std::string rpl_namreply(const std::string& server, const std::string& nick,
	                         const std::string& channel, const std::string& names);

	// RPL_ENDOFNAMES (366) — terminates the NAMES / JOIN name list.
	std::string rpl_endofnames(const std::string& server, const std::string& nick,
	                           const std::string& channel);

	// RPL_WHOREPLY (352) — one line of a WHO reply.
	std::string rpl_whoreply(const std::string& server, const std::string& nick,
	                         const Client& member, const std::string& channel,
	                         const std::string& flags);

	// RPL_ENDOFWHO (315) — terminates a WHO reply.
	std::string rpl_endofwho(const std::string& server, const std::string& nick,
	                         const std::string& name);

	// RPL_ENDOFBANLIST (368) — empty ban/exception/invite-exempt list.
	std::string rpl_endofbanlist(const std::string& server, const std::string& nick,
	                             const std::string& channel);

	// ── Error numerics ────────────────────────────────────────────────────────

	// ERR_NOSUCHNICK (401) — target of PRIVMSG, NOTICE, KICK, or INVITE does
	//   not match any connected client or known channel.
	std::string err_nosuchnick(const std::string& server, const std::string& nick,
	                           const std::string& target);

	// ERR_NOSUCHCHANNEL (403) — channel name in JOIN, PART, TOPIC, KICK, etc.
	//   does not exist or is syntactically invalid.
	std::string err_nosuchchannel(const std::string& server, const std::string& nick,
	                              const std::string& channel);

	// ERR_CANNOTSENDTOCHAN (404) — PRIVMSG or NOTICE sent to a channel by a
	//   client that is not a member of it (no external messages allowed).
	std::string err_cannotsendtochan(const std::string& server, const std::string& nick,
	                                 const std::string& channel);

	// ERR_NORECIPIENT (411) — PRIVMSG sent with no target argument.
	std::string err_norecipient(const std::string& server, const std::string& nick,
	                            const std::string& cmd);

	// ERR_NOTEXTTOSEND (412) — PRIVMSG or NOTICE sent with an empty trailing
	//   (the message body is missing entirely).
	std::string err_notexttosend(const std::string& server, const std::string& nick);

	// ERR_NOORIGIN (409) — PING received with no token/origin argument.
	std::string err_noorigin(const std::string& server, const std::string& nick);

	// ERR_UNKNOWNCOMMAND (421) — command token not recognised after parsing.
	//   Send from cmd_unknown; use "*" as the nick if the client is not yet
	//   registered.
	std::string err_unknowncommand(const std::string& server, const std::string& nick,
	                               const std::string& cmd);

	// ERR_NONICKNAMEGIVEN (431) — NICK received with no argument.
	std::string err_nonicknamegiven(const std::string& server, const std::string& nick);

	// ERR_ERRONEUSNICKNAME (432) — NICK argument contains illegal characters
	//   or exceeds the 9-character limit.
	std::string err_erroneusnickname(const std::string& server, const std::string& nick,
	                                 const std::string& bad_nick);

	// ERR_NICKNAMEINUSE (433) — NICK argument is already held by another client.
	std::string err_nicknameinuse(const std::string& server, const std::string& nick,
	                              const std::string& taken_nick);

	// ERR_NOTONCHANNEL (442) — PART, TOPIC, or KICK attempted on a channel the
	//   issuing client is not currently a member of.
	std::string err_notonchannel(const std::string& server, const std::string& nick,
	                             const std::string& channel);

	// ERR_USERONCHANNEL (443) — INVITE target is already a member of the channel.
	std::string err_useronchannel(const std::string& server, const std::string& nick,
	                              const std::string& target, const std::string& channel);

	// ERR_NEEDMOREPARAMS (461) — command received with fewer arguments than the
	//   minimum required (PASS, NICK, USER, JOIN, PART, KICK, INVITE, MODE, ...).
	std::string err_needmoreparams(const std::string& server, const std::string& nick,
	                               const std::string& cmd);

	// ERR_ALREADYREGISTRED (462) — PASS or USER received after the client has
	//   already completed registration.
	std::string err_alreadyregistred(const std::string& server, const std::string& nick);

	// ERR_NOTREGISTERED (451) — command requiring registration received before
	//   PASS + NICK + USER have all completed.
	std::string err_notregistered(const std::string& server, const std::string& nick);

	// ERR_PASSWDMISMATCH (464) — PASS argument does not match the server password.
	//   Send before closing the connection or when finalizing registration.
	std::string err_passwdmismatch(const std::string& server, const std::string& nick);

	// ERR_CHANNELISFULL (471) — JOIN refused because the channel's user limit
	//   (+l) has been reached.
	std::string err_channelisfull(const std::string& server, const std::string& nick,
	                              const std::string& channel);

	// ERR_INVITEONLYCHAN (473) — JOIN refused because the channel is invite-only
	//   (+i) and the client has not received an INVITE.
	std::string err_inviteonlychan(const std::string& server, const std::string& nick,
	                               const std::string& channel);

	// ERR_BADCHANNELKEY (475) — JOIN refused because the supplied key does not
	//   match the channel key (+k).
	std::string err_badchannelkey(const std::string& server, const std::string& nick,
	                              const std::string& channel);

	// ERR_CHANOPRIVSNEEDED (482) — KICK, MODE, or TOPIC (when +t is set) refused
	//   because the issuing client is not a channel operator.
	std::string err_chanoprivsneeded(const std::string& server, const std::string& nick,
	                                 const std::string& channel);

	// ERR_USERSDONTMATCH (502) — user MODE attempted on another client's nick.
	std::string err_usersdontmatch(const std::string& server, const std::string& nick);

	// ERR_USERNOTINCHANNEL (441) — KICK target is not a member of the channel.
	std::string err_usernotinchannel(const std::string& server, const std::string& nick,
	                                 const std::string& target, const std::string& channel);

	// RPL_CHANNELMODEIS (324) — reply to MODE query with no modestring argument.
	std::string rpl_channelmodeis(const std::string& server, const std::string& nick,
	                              const std::string& channel, const std::string& modestring);

	// ERR_UNKNOWNMODE (472) — MODE modestring contains an unrecognised character.
	std::string err_unknownmode(const std::string& server, const std::string& nick,
	                            char modechar);

} // namespace Reply

#endif
