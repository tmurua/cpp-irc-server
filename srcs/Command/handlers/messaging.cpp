/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   messaging.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/15 18:16:03 by johartma          #+#    #+#             */
/*   Updated: 2026/06/15 18:16:06 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command/Handlers.hpp"
#include "Server.hpp"
#include "Reply.hpp"
#include <vector>
#include <string>
#include <set>

static std::vector<std::string> split(const std::string& s, char delim)
{
	std::vector<std::string> result;
	std::string token;
	for (std::size_t i = 0; i < s.size(); i++) {
		if (s[i] == delim) {
			if (!token.empty()) { result.push_back(token); token.clear(); }
		} else {
			token += s[i];
		}
	}
	if (!token.empty()) result.push_back(token);
	return result;
}

static void dispatch(Server& serv, Client& sender,
                     const std::string& target, const std::string& cmd,
                     const std::string& text, bool silent)
{
	Clients&  clients  = serv.getClients();
	Channels& channels = serv.getChannels();

	std::string msg = Reply::from_client(sender, cmd, target, text);

	if (!target.empty() && (target[0] == '#' || target[0] == '&')) {
		Channel* chan = channels.get_channel(target);
		if (!chan) {
			if (!silent)
				serv.sndmsg(sender, Reply::err_nosuchchannel("", sender.getNickname(), target));
			return;
		}
		if (!chan->hasUser(sender.getNickname())) {
			if (!silent)
				serv.sndmsg(sender, Reply::err_cannotsendtochan("", sender.getNickname(), target));
			return;
		}
		const std::set<std::string>& users = chan->getUsers();
		for (std::set<std::string>::const_iterator it = users.begin(); it != users.end(); ++it) {
			Client* m = clients.get_client(*it);
			if (m && m->getFd() != sender.getFd())
				serv.sndmsg(*m, msg);
		}
	} else {
		Client* dest = clients.get_client(target);
		if (!dest) {
			if (!silent)
				serv.sndmsg(sender, Reply::err_nosuchnick("", sender.getNickname(), target));
			return;
		}
		serv.sndmsg(*dest, msg);
	}
}

int	cmd_privmsg(Command& cmd, Server& serv) {
	Clients& clients = serv.getClients();
	Client*  client  = clients.get_client_fd(cmd.getFd());
	if (!client) return 1;
	if (!client->isRegistered()) {
		serv.sndmsg(*client, Reply::err_notregistered("", client->getNickname()));
		return 1;
	}

	const std::string& nick = client->getNickname();
	if (cmd.getArguments().empty()) {
		serv.sndmsg(*client, Reply::err_norecipient("", nick, "PRIVMSG"));
		return 1;
	}
	if (cmd.getTrailing().empty()) {
		serv.sndmsg(*client, Reply::err_notexttosend("", nick));
		return 1;
	}

	std::vector<std::string> targets = split(cmd.getArguments()[0], ',');
	for (std::size_t i = 0; i < targets.size(); i++)
		dispatch(serv, *client, targets[i], "PRIVMSG", cmd.getTrailing(), false);
	return 0;
}

int	cmd_notice(Command& cmd, Server& serv) {
	Clients& clients = serv.getClients();
	Client*  client  = clients.get_client_fd(cmd.getFd());
	if (!client || !client->isRegistered()) return 0;

	if (cmd.getArguments().empty() || cmd.getTrailing().empty()) return 0;

	std::vector<std::string> targets = split(cmd.getArguments()[0], ',');
	for (std::size_t i = 0; i < targets.size(); i++)
		dispatch(serv, *client, targets[i], "NOTICE", cmd.getTrailing(), true);
	return 0;
}
