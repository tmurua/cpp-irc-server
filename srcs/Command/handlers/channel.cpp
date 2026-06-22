/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/15 18:15:48 by johartma          #+#    #+#             */
/*   Updated: 2026/06/15 18:15:52 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command/Handlers.hpp"
#include "Server.hpp"
#include "Reply.hpp"
#include <vector>
#include <string>
#include <set>
#include <cstdlib>

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

static void broadcast(Server& serv, Channel* chan, const std::string& msg, int skip_fd = -1)
{
	const std::set<std::string>& users = chan->getUsers();
	for (std::set<std::string>::const_iterator it = users.begin(); it != users.end(); ++it) {
		Client* m = serv.getClients().get_client(*it);
		if (m && m->getFd() != skip_fd)
			serv.sndmsg(*m, msg);
	}
}

static bool valid_channame(const std::string& name)
{
	return !name.empty()
		&& (name[0] == '#' || name[0] == '&')
		&& name.size() <= 200;
}

static bool is_mode_list_query(const std::string& token)
{
	return token == "b" || token == "+b" || token == "e" || token == "+e"
		|| token == "I" || token == "+I";
}

int	cmd_join(Command& cmd, Server& serv) {
	Clients&  clients  = serv.getClients();
	Channels& channels = serv.getChannels();
	Client*   client   = clients.get_client_fd(cmd.getFd());

	if (!client) return 1;
	if (!client->isRegistered()) {
		serv.sndmsg(*client, Reply::err_notregistered(serv.getServerName(), client->getNickname()));
		return 1;
	}
	if (cmd.getArguments().empty()) {
		serv.sndmsg(*client, Reply::err_needmoreparams(serv.getServerName(), client->getNickname(), "JOIN"));
		return 1;
	}

	std::vector<std::string> channames = split(cmd.getArguments()[0], ',');
	std::vector<std::string> keys;
	if (cmd.getArguments().size() > 1)
		keys = split(cmd.getArguments()[1], ',');

	const std::string& nick = client->getNickname();

	for (std::size_t i = 0; i < channames.size(); i++) {
		const std::string& channame = channames[i];
		std::string        key      = (i < keys.size()) ? keys[i] : "";

		if (!valid_channame(channame)) {
			serv.sndmsg(*client, Reply::err_nosuchchannel(serv.getServerName(), nick, channame));
			continue;
		}

		Channel* chan = channels.get_channel(channame);
		if (chan && chan->hasUser(nick))
			continue;
		if (!chan) {
			Channel newchan(channame);
			channels.add_channel(newchan);
			chan = channels.get_channel(channame);
			chan->addOp(nick);
		} else {
			if (chan->isInviteOnly() && !chan->isInvited(nick)) {
				serv.sndmsg(*client, Reply::err_inviteonlychan(serv.getServerName(), nick, channame));
				continue;
			}
			if (chan->hasKey() && chan->getKey() != key) {
				serv.sndmsg(*client, Reply::err_badchannelkey(serv.getServerName(), nick, channame));
				continue;
			}
			if (chan->getUserLimit() > 0
					&& static_cast<int>(chan->getUsers().size()) >= chan->getUserLimit()) {
				serv.sndmsg(*client, Reply::err_channelisfull(serv.getServerName(), nick, channame));
				continue;
			}
		}

		chan->addUser(nick);
		client->joinChannel(channame);
		chan->clearInvite(nick);

		broadcast(serv, chan, Reply::from_client(*client, "JOIN", channame));

		if (chan->hasTopic())
			serv.sndmsg(*client, Reply::rpl_topic(serv.getServerName(), nick, channame, chan->getTopic()));
		else
			serv.sndmsg(*client, Reply::rpl_notopic(serv.getServerName(), nick, channame));

		std::string namesList;
		const std::set<std::string>& users = chan->getUsers();
		for (std::set<std::string>::const_iterator it = users.begin(); it != users.end(); ++it) {
			if (!namesList.empty()) namesList += " ";
			if (chan->isOp(*it)) namesList += "@";
			namesList += *it;
		}
		serv.sndmsg(*client, Reply::rpl_namreply(serv.getServerName(), nick, channame, namesList));
		serv.sndmsg(*client, Reply::rpl_endofnames(serv.getServerName(), nick, channame));
	}
	return 0;
}

int	cmd_part(Command& cmd, Server& serv) {
	Clients&  clients  = serv.getClients();
	Channels& channels = serv.getChannels();
	Client*   client   = clients.get_client_fd(cmd.getFd());

	if (!client) return 1;
	if (!client->isRegistered()) {
		serv.sndmsg(*client, Reply::err_notregistered(serv.getServerName(), client->getNickname()));
		return 1;
	}
	if (cmd.getArguments().empty()) {
		serv.sndmsg(*client, Reply::err_needmoreparams(serv.getServerName(), client->getNickname(), "PART"));
		return 1;
	}

	std::vector<std::string> channames = split(cmd.getArguments()[0], ',');
	const std::string& nick   = client->getNickname();
	std::string        reason = cmd.getTrailing().empty() ? nick : cmd.getTrailing();

	for (std::size_t i = 0; i < channames.size(); i++) {
		const std::string& channame = channames[i];
		Channel* chan = channels.get_channel(channame);
		if (!chan) {
			serv.sndmsg(*client, Reply::err_nosuchchannel(serv.getServerName(), nick, channame));
			continue;
		}
		if (!chan->hasUser(nick)) {
			serv.sndmsg(*client, Reply::err_notonchannel(serv.getServerName(), nick, channame));
			continue;
		}
		broadcast(serv, chan, Reply::from_client(*client, "PART", channame, reason));
		chan->removeUser(nick);
		client->leaveChannel(channame);
		if (chan->getUsers().empty())
			channels.remove_channel(channame);
	}
	return 0;
}

int	cmd_topic(Command& cmd, Server& serv) {
	Clients&  clients  = serv.getClients();
	Channels& channels = serv.getChannels();
	Client*   client   = clients.get_client_fd(cmd.getFd());

	if (!client) return 1;
	if (!client->isRegistered()) {
		serv.sndmsg(*client, Reply::err_notregistered(serv.getServerName(), client->getNickname()));
		return 1;
	}
	if (cmd.getArguments().empty()) {
		serv.sndmsg(*client, Reply::err_needmoreparams(serv.getServerName(), client->getNickname(), "TOPIC"));
		return 1;
	}

	const std::string& nick    = client->getNickname();
	const std::string& channame = cmd.getArguments()[0];
	Channel* chan = channels.get_channel(channame);
	if (!chan) {
		serv.sndmsg(*client, Reply::err_nosuchchannel(serv.getServerName(), nick, channame));
		return 1;
	}
	if (!chan->hasUser(nick)) {
		serv.sndmsg(*client, Reply::err_notonchannel(serv.getServerName(), nick, channame));
		return 1;
	}

	// Query: single arg with no trailing token at all
	if (cmd.getArguments().size() == 1 && !cmd.hasTrailing()) {
		if (chan->hasTopic())
			serv.sndmsg(*client, Reply::rpl_topic(serv.getServerName(), nick, channame, chan->getTopic()));
		else
			serv.sndmsg(*client, Reply::rpl_notopic(serv.getServerName(), nick, channame));
		return 0;
	}

	// Set / clear (explicit empty trailing means clear)
	if (chan->isTopicRestricted() && !chan->isOp(nick)) {
		serv.sndmsg(*client, Reply::err_chanoprivsneeded(serv.getServerName(), nick, channame));
		return 1;
	}
	const std::string& newTopic = cmd.getTrailing();
	if (newTopic.empty()) {
		chan->clearTopic();
		// Explicitly include " :" so receivers distinguish a clear from a query.
		std::string clearMsg = ":" + client->getNickname() + "!" + client->getUsername()
		                     + "@" + client->getHostname()
		                     + " TOPIC " + channame + " :\r\n";
		broadcast(serv, chan, clearMsg);
	} else {
		chan->setTopic(newTopic);
		broadcast(serv, chan, Reply::from_client(*client, "TOPIC", channame, newTopic));
	}
	return 0;
}

int	cmd_kick(Command& cmd, Server& serv) {
	Clients&  clients  = serv.getClients();
	Channels& channels = serv.getChannels();
	Client*   client   = clients.get_client_fd(cmd.getFd());

	if (!client) return 1;
	if (!client->isRegistered()) {
		serv.sndmsg(*client, Reply::err_notregistered(serv.getServerName(), client->getNickname()));
		return 1;
	}
	if (cmd.getArguments().size() < 2) {
		serv.sndmsg(*client, Reply::err_needmoreparams(serv.getServerName(), client->getNickname(), "KICK"));
		return 1;
	}

	const std::string& nick    = client->getNickname();
	const std::string& channame = cmd.getArguments()[0];
	std::string        reason  = cmd.getTrailing().empty() ? nick : cmd.getTrailing();

	Channel* chan = channels.get_channel(channame);
	if (!chan) {
		serv.sndmsg(*client, Reply::err_nosuchchannel(serv.getServerName(), nick, channame));
		return 1;
	}
	if (!chan->hasUser(nick)) {
		serv.sndmsg(*client, Reply::err_notonchannel(serv.getServerName(), nick, channame));
		return 1;
	}
	if (!chan->isOp(nick)) {
		serv.sndmsg(*client, Reply::err_chanoprivsneeded(serv.getServerName(), nick, channame));
		return 1;
	}

	std::vector<std::string> targets = split(cmd.getArguments()[1], ',');
	for (std::size_t i = 0; i < targets.size(); i++) {
		const std::string& target = targets[i];
		if (!chan->hasUser(target)) {
			serv.sndmsg(*client, Reply::err_usernotinchannel(serv.getServerName(), nick, target, channame));
			continue;
		}
		broadcast(serv, chan,
			Reply::from_client(*client, "KICK", channame + " " + target, reason));
		Client* targetClient = clients.get_client(target);
		chan->removeUser(target);
		if (targetClient) targetClient->leaveChannel(channame);
		if (chan->getUsers().empty()) {
			channels.remove_channel(channame);
			break;
		}
	}
	return 0;
}

int	cmd_invite(Command& cmd, Server& serv) {
	Clients&  clients  = serv.getClients();
	Channels& channels = serv.getChannels();
	Client*   client   = clients.get_client_fd(cmd.getFd());

	if (!client) return 1;
	if (!client->isRegistered()) {
		serv.sndmsg(*client, Reply::err_notregistered(serv.getServerName(), client->getNickname()));
		return 1;
	}
	if (cmd.getArguments().size() < 2) {
		serv.sndmsg(*client, Reply::err_needmoreparams(serv.getServerName(), client->getNickname(), "INVITE"));
		return 1;
	}

	const std::string& nick       = client->getNickname();
	const std::string& targetNick = cmd.getArguments()[0];
	const std::string& channame   = cmd.getArguments()[1];

	Client* target = clients.get_client(targetNick);
	if (!target) {
		serv.sndmsg(*client, Reply::err_nosuchnick(serv.getServerName(), nick, targetNick));
		return 1;
	}
	Channel* chan = channels.get_channel(channame);
	if (!chan) {
		serv.sndmsg(*client, Reply::err_nosuchchannel(serv.getServerName(), nick, channame));
		return 1;
	}
	if (!chan->hasUser(nick)) {
		serv.sndmsg(*client, Reply::err_notonchannel(serv.getServerName(), nick, channame));
		return 1;
	}
	if (chan->isInviteOnly() && !chan->isOp(nick)) {
		serv.sndmsg(*client, Reply::err_chanoprivsneeded(serv.getServerName(), nick, channame));
		return 1;
	}
	if (chan->hasUser(targetNick)) {
		serv.sndmsg(*client, Reply::err_useronchannel(serv.getServerName(), nick, targetNick, channame));
		return 1;
	}

	chan->addInvite(targetNick);
	serv.sndmsg(*client, Reply::rpl_inviting(serv.getServerName(), nick, targetNick, channame));
	serv.sndmsg(*target, Reply::from_client(*client, "INVITE", targetNick, channame));
	return 0;
}

int	cmd_mode(Command& cmd, Server& serv) {
	Clients&  clients  = serv.getClients();
	Channels& channels = serv.getChannels();
	Client*   client   = clients.get_client_fd(cmd.getFd());

	if (!client) return 1;
	if (!client->isRegistered()) {
		serv.sndmsg(*client, Reply::err_notregistered(serv.getServerName(), client->getNickname()));
		return 1;
	}
	if (cmd.getArguments().empty()) {
		serv.sndmsg(*client, Reply::err_needmoreparams(serv.getServerName(), client->getNickname(), "MODE"));
		return 1;
	}

	const std::string& nick    = client->getNickname();
	const std::string& target  = cmd.getArguments()[0];

	if (target.empty() || (target[0] != '#' && target[0] != '&')) {
		if (target != nick)
			serv.sndmsg(*client, Reply::err_usersdontmatch(serv.getServerName(), nick));
		return 0;
	}

	const std::string& channame = target;
	Channel* chan = channels.get_channel(channame);
	if (!chan) {
		serv.sndmsg(*client, Reply::err_nosuchchannel(serv.getServerName(), nick, channame));
		return 1;
	}

	// Query: no modestring
	if (cmd.getArguments().size() == 1 && cmd.getTrailing().empty()) {
		std::string modestr = "+";
		if (chan->isInviteOnly())      modestr += "i";
		if (chan->isTopicRestricted()) modestr += "t";
		if (chan->hasKey())            modestr += "k";
		if (chan->getUserLimit() > 0)  modestr += "l";
		serv.sndmsg(*client, Reply::rpl_channelmodeis(serv.getServerName(), nick, channame, modestr));
		return 0;
	}

	if (cmd.getArguments().size() == 2 && cmd.getTrailing().empty()
			&& is_mode_list_query(cmd.getArguments()[1])) {
		if (!chan->hasUser(nick)) {
			serv.sndmsg(*client, Reply::err_notonchannel(serv.getServerName(), nick, channame));
			return 1;
		}
		serv.sndmsg(*client, Reply::rpl_endofbanlist(serv.getServerName(), nick, channame));
		return 0;
	}

	if (!chan->isOp(nick)) {
		serv.sndmsg(*client, Reply::err_chanoprivsneeded(serv.getServerName(), nick, channame));
		return 1;
	}

	// modestring is args[1] when present, otherwise the trailing (e.g. "MODE #chan :+i")
	const std::string& modestr = (cmd.getArguments().size() >= 2)
		? cmd.getArguments()[1] : cmd.getTrailing();
	// Gather mode arguments (args[2..] + trailing if modestr came from args[1])
	std::vector<std::string> modeargs;
	if (cmd.getArguments().size() >= 2) {
		modeargs.assign(cmd.getArguments().begin() + 2, cmd.getArguments().end());
		if (!cmd.getTrailing().empty()) modeargs.push_back(cmd.getTrailing());
	}

	char        sign       = '+';
	std::size_t argIdx     = 0;
	// Seed the implicit leading '+' only when the modestring has no explicit sign prefix;
	// sign changes append to applied rather than replacing it so earlier letters are kept.
	std::string applied    = (!modestr.empty() && modestr[0] != '+' && modestr[0] != '-') ? "+" : "";
	std::string appliedArgs;

	for (std::size_t i = 0; i < modestr.size(); i++) {
		char c = modestr[i];
		if (c == '+') { sign = '+'; applied += '+'; continue; }
		if (c == '-') { sign = '-'; applied += '-'; continue; }

		switch (c) {
		case 'i':
			chan->setInviteOnly(sign == '+');
			applied += 'i';
			break;
		case 't':
			chan->setTopicRestricted(sign == '+');
			applied += 't';
			break;
		case 'k':
			if (sign == '+' && argIdx < modeargs.size()) {
				chan->setKey(modeargs[argIdx]);
				if (!appliedArgs.empty()) appliedArgs += " ";
				appliedArgs += modeargs[argIdx++];
				applied += 'k';
			} else if (sign == '-') {
				chan->clearKey();
				applied += 'k';
			}
			break;
		case 'o':
			if (argIdx < modeargs.size()) {
				const std::string& tgt = modeargs[argIdx];
				if (!chan->hasUser(tgt)) {
					serv.sndmsg(*client,
						Reply::err_usernotinchannel(serv.getServerName(), nick, tgt, channame));
					argIdx++;
					break;
				}
				if (sign == '+') chan->addOp(tgt);
				else             chan->removeOp(tgt);
				if (!appliedArgs.empty()) appliedArgs += " ";
				appliedArgs += tgt;
				argIdx++;
				applied += 'o';
			}
			break;
		case 'l':
			if (sign == '+' && argIdx < modeargs.size()) {
				chan->setUserLimit(std::atoi(modeargs[argIdx].c_str()));
				if (!appliedArgs.empty()) appliedArgs += " ";
				appliedArgs += modeargs[argIdx++];
				applied += 'l';
			} else if (sign == '-') {
				chan->setUserLimit(0);
				applied += 'l';
			}
			break;
		default:
			serv.sndmsg(*client, Reply::err_unknownmode(serv.getServerName(), nick, c));
			break;
		}
	}

	bool anyApplied = false;
	for (std::size_t i = 0; i < applied.size(); i++) {
		if (applied[i] != '+' && applied[i] != '-') { anyApplied = true; break; }
	}
	if (anyApplied) {
		std::string relay = channame + " " + applied;
		if (!appliedArgs.empty()) relay += " " + appliedArgs;
		broadcast(serv, chan, Reply::from_client(*client, "MODE", relay));
	}
	return 0;
}
