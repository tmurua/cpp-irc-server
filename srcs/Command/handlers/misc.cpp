/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   misc.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/15 16:34:53 by johartma          #+#    #+#             */
/*   Updated: 2026/06/15 17:38:17 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command/Handlers.hpp"
#include "Server.hpp"
#include "Reply.hpp"
#include "Clients/Clients.hpp"
#include "Clients/Client.hpp"
#include "Channels/Channels.hpp"
#include "Channels/Channel.hpp"
#include <set>
#include <string>

static Client* get_or_create(int fd, Server& serv)
{
	Clients& clients = serv.getClients();
	Client*  cl      = clients.get_client_fd(fd);

	if (!cl) {
		Client tmp(fd);
		tmp.setHostname(serv.getPeerHostname(fd));
		clients.add_client(tmp);
		cl = clients.get_client_fd(fd);
	}
	return cl;
}

int	cmd_ping(Command& cmd, Server& serv) {
	Client* client = serv.getClients().get_client_fd(cmd.getFd());
	if (!client) return 1;

	if (cmd.getArguments().empty() && cmd.getTrailing().empty()) {
		const std::string& server = serv.getServerName();
		serv.sndmsg(*client, Reply::from_server(server, "PONG", server));
		return 0;
	}
	std::string token = cmd.getArguments().empty()
		? cmd.getTrailing() : cmd.getArguments()[0];
	serv.sndmsg(*client, Reply::from_server(serv.getServerName(), "PONG", token));
	return 0;
}

int	cmd_pong(Command& cmd, Server& serv) {
	(void)cmd;
	(void)serv;
	return 0;
}

int	cmd_cap(Command& cmd, Server& serv) {
	Client* client = get_or_create(cmd.getFd(), serv);
	if (!client)
		return 1;
	if (cmd.getArguments().empty())
		return 0;

	const std::string& sub = cmd.getArguments()[0];
	const std::string& server = serv.getServerName();

	if (sub == "LS") {
		serv.sndmsg(*client, ":" + server + " CAP * LS :\r\n");
	} else if (sub == "REQ") {
		serv.sndmsg(*client, ":" + server + " CAP * NAK :\r\n");
	}
	return 0;
}

static void send_whoreply(Server& serv, Client& requester, Client& member,
                          const std::string& channel, bool is_op)
{
	std::string flags = "H";
	if (is_op)
		flags += "@";
	serv.sndmsg(requester, Reply::rpl_whoreply(serv.getServerName(),
		requester.getNickname(), member, channel, flags));
}

int	cmd_who(Command& cmd, Server& serv) {
	Clients&  clients  = serv.getClients();
	Channels& channels = serv.getChannels();
	Client*   client   = clients.get_client_fd(cmd.getFd());

	if (!client || !client->isRegistered())
		return 0;

	const std::string& nick   = client->getNickname();
	const std::string& server = serv.getServerName();
	std::string        query  = cmd.getArguments().empty() ? "" : cmd.getArguments()[0];

	if (query.empty()) {
		serv.sndmsg(*client, Reply::rpl_endofwho(server, nick, "*"));
		return 0;
	}

	if (query[0] == '#' || query[0] == '&') {
		Channel* chan = channels.get_channel(query);
		if (chan) {
			const std::set<std::string>& users = chan->getUsers();
			for (std::set<std::string>::const_iterator it = users.begin();
			     it != users.end(); ++it) {
				Client* member = clients.get_client(*it);
				if (member)
					send_whoreply(serv, *client, *member, query, chan->isOp(*it));
			}
		}
		serv.sndmsg(*client, Reply::rpl_endofwho(server, nick, query));
		return 0;
	}

	Client* target = clients.get_client(query);
	if (target)
		send_whoreply(serv, *client, *target, "*", false);
	serv.sndmsg(*client, Reply::rpl_endofwho(server, nick, query));
	return 0;
}

int	cmd_unknown(Command& cmd, Server& serv) {
	Client* client = serv.getClients().get_client_fd(cmd.getFd());
	std::string nick = client ? client->getNickname() : "*";
	if (client)
		serv.sndmsg(*client, Reply::err_unknowncommand(serv.getServerName(), nick, cmd.getRawCmd()));
	return 0;
}
