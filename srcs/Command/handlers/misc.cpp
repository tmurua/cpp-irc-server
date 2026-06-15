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

int	cmd_ping(Command& cmd, Server& serv) {
	Client* client = serv.getClients().get_client_fd(cmd.getFd());
	if (!client) return 1;

	if (cmd.getArguments().empty() && cmd.getTrailing().empty()) {
		serv.sndmsg(*client, Reply::err_noorigin("", client->getNickname()));
		return 1;
	}
	std::string token = cmd.getArguments().empty()
		? cmd.getTrailing() : cmd.getArguments()[0];
	serv.sndmsg(*client, Reply::from_server("", "PONG", "", token));
	return 0;
}

int	cmd_pong(Command& cmd, Server& serv) {
	(void)cmd;
	(void)serv;
	return 0;
}

int	cmd_unknown(Command& cmd, Server& serv) {
	Client* client = serv.getClients().get_client_fd(cmd.getFd());
	std::string nick = client ? client->getNickname() : "*";
	if (client)
		serv.sndmsg(*client, Reply::err_unknowncommand("", nick, cmd.getRawCmd()));
	return 0;
}
