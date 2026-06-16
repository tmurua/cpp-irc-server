/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   auth.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/15 18:15:35 by johartma          #+#    #+#             */
/*   Updated: 2026/06/15 18:15:39 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command/Handlers.hpp"
#include "Server.hpp"
#include "Reply.hpp"
#include "Channels/Channels.hpp"
#include <cctype>
#include <set>
#include <string>

static void register_try(Client *cl, Server& serv)
{
	if (cl->isRegistered() || !cl->isAuthenticated()
			|| cl->getNickname().empty() || cl->getUsername().empty())
		return;
	cl->setRegistered(true);
	serv.sndmsg(*cl, Reply::rpl_welcome("", *cl));
	serv.sndmsg(*cl, Reply::rpl_yourhost("", *cl));
	serv.sndmsg(*cl, Reply::rpl_created("", *cl));
	serv.sndmsg(*cl, Reply::rpl_myinfo("", *cl));
}

static bool is_valid_nick(const std::string& nick)
{
	static const std::string special = "[]\\`_^{|}";
	if (nick.empty() || nick.size() > 9)
		return false;
	if (!std::isalpha(static_cast<unsigned char>(nick[0])) && special.find(nick[0]) == std::string::npos)
		return false;
	for (std::size_t i = 1; i < nick.size(); i++) {
		char c = nick[i];
		if (!std::isalnum(static_cast<unsigned char>(c)) && c != '-' && special.find(c) == std::string::npos)
			return false;
	}
	return true;
}

static Client* get_or_create(int fd, Clients& clients)
{
	Client* cl = clients.get_client_fd(fd);
	if (!cl) {
		Client tmp(fd);
		clients.add_client(tmp);
		cl = clients.get_client_fd(fd);
	}
	return cl;
}

int	cmd_pass(Command& cmd, Server& serv) {
	Clients& clients = serv.getClients();
	Client*  client  = get_or_create(cmd.getFd(), clients);

	if (cmd.getArguments().empty()) {
		serv.sndmsg(*client, Reply::err_needmoreparams("", client->getNickname(), "PASS"));
		return 1;
	}
	if (client->isRegistered()) {
		serv.sndmsg(*client, Reply::err_alreadyregistred("", client->getNickname()));
		return 3;
	}
	if (cmd.getArguments()[0] != serv.getPassword()) {
		serv.sndmsg(*client, Reply::err_passwdmismatch("", client->getNickname()));
		return 4;
	}
	client->setAuthenticated(true);
	register_try(client, serv);
	return 0;
}

int	cmd_nick(Command& cmd, Server& serv) {
	Clients& clients = serv.getClients();
	Client*  client  = get_or_create(cmd.getFd(), clients);

	std::string oldnick = client->getNickname();

	if (cmd.getArguments().empty()) {
		serv.sndmsg(*client, Reply::err_nonicknamegiven("", oldnick));
		return 2;
	}

	const std::string& newnick = cmd.getArguments()[0];
	if (newnick == oldnick) {
		register_try(client, serv);
		return 0;
	}
	if (!is_valid_nick(newnick)) {
		serv.sndmsg(*client, Reply::err_erroneusnickname("", oldnick, newnick));
		return 3;
	}
	if (clients.get_client(newnick)) {
		serv.sndmsg(*client, Reply::err_nicknameinuse("", oldnick, newnick));
		return 4;
	}

	// Build relay before updating so the old prefix is used
	std::string relayMsg = Reply::from_client(*client, "NICK", "", newnick);

	clients.update_nick(cmd.getFd(), newnick);

	if (client->isRegistered()) {
		Channels& channels = serv.getChannels();
		const std::set<std::string> chanList = client->getChannels();
		for (std::set<std::string>::const_iterator it = chanList.begin(); it != chanList.end(); ++it) {
			Channel* chan = channels.get_channel(*it);
			if (!chan) continue;
			bool wasOp = chan->isOp(oldnick);
			chan->removeUser(oldnick);
			chan->addUser(newnick);
			if (wasOp) chan->addOp(newnick);
			const std::set<std::string>& users = chan->getUsers();
			for (std::set<std::string>::const_iterator uit = users.begin(); uit != users.end(); ++uit) {
				Client* m = clients.get_client(*uit);
				// Skip self here; we send exactly once below after all channels.
				if (m && m->getFd() != cmd.getFd())
					serv.sndmsg(*m, relayMsg);
			}
		}
		// Migrate pending invite-list entries on every channel so a renamed
		// user can still JOIN an invite-only channel they were invited to.
		const std::map<std::string, Channel>& all = channels.getAll();
		for (std::map<std::string, Channel>::const_iterator it = all.begin();
		     it != all.end(); ++it) {
			Channel* c = channels.get_channel(it->first);
			if (c && c->isInvited(oldnick)) {
				c->clearInvite(oldnick);
				c->addInvite(newnick);
			}
		}
		// Send NICK relay to the renaming client exactly once.
		serv.sndmsg(*client, relayMsg);
	}

	register_try(client, serv);
	return 0;
}

int	cmd_user(Command& cmd, Server& serv) {
	Clients& clients = serv.getClients();
	Client*  client  = get_or_create(cmd.getFd(), clients);

	if (cmd.getArguments().size() < 3 || cmd.getTrailing().empty()) {
		serv.sndmsg(*client, Reply::err_needmoreparams("", client->getNickname(), "USER"));
		return 1;
	}
	if (client->isRegistered()) {
		serv.sndmsg(*client, Reply::err_alreadyregistred("", client->getNickname()));
		return 2;
	}
	// Per RFC 2812, the <hostname> and <servername> parameters from a client
	// USER command are ignored — the server uses the values it determined at
	// connection time. Trusting them lets a client spoof its prefix.
	client->setUsername(cmd.getArguments()[0]);
	client->setRealname(cmd.getTrailing());
	register_try(client, serv);
	return 0;
}

int	cmd_quit(Command& cmd, Server& serv) {
	Clients&    clients  = serv.getClients();
	Channels&   channels = serv.getChannels();
	Client*     client   = clients.get_client_fd(cmd.getFd());

	std::string reason = cmd.getTrailing().empty() ? "Client Quit" : cmd.getTrailing();

	if (client) {
		std::string nick     = client->getNickname();
		std::string quitMsg  = Reply::from_client(*client, "QUIT", "", reason);

		// Broadcast QUIT to every channel, then remove the client from it
		std::set<std::string> chanList = client->getChannels();
		for (std::set<std::string>::const_iterator it = chanList.begin(); it != chanList.end(); ++it) {
			Channel* chan = channels.get_channel(*it);
			if (!chan) continue;
			const std::set<std::string> users = chan->getUsers();
			for (std::set<std::string>::const_iterator uit = users.begin(); uit != users.end(); ++uit) {
				if (*uit == nick) continue;
				Client* m = clients.get_client(*uit);
				if (m) serv.sndmsg(*m, quitMsg);
			}
			chan->removeUser(nick);
			if (chan->getUsers().empty())
				channels.remove_channel(*it);
		}
		serv.sndmsg(*client, "ERROR :Closing Link: " + reason + "\r\n");
	}
	serv.disconnectClient(cmd.getFd());
	return 0;
}
