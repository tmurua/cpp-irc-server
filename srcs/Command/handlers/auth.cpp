/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   auth.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 00:00:00 by johartma          #+#    #+#             */
/*   Updated: 2026/06/09 00:00:00 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command/Handlers.hpp"
#include "Server.hpp"
#include "Reply.hpp"
#include <cctype>

static void register_try(Client *cl, Server& serv)
{
	if (cl->isRegistered() || !cl->isAuthenticated()  
		|| cl->getNickname().empty() || cl->getUsername().empty())
		return;
	cl->setRegistered(true);
	serv.sndmsg(cl->getFd(), Reply::rpl_welcome("", *cl));
	serv.sndmsg(cl->getFd(), Reply::rpl_yourhost("", *cl));
	serv.sndmsg(cl->getFd(), Reply::rpl_welcome("", *cl));
	serv.sndmsg(cl->getFd(), Reply::rpl_myinfo("", *cl));
}

static bool	is_valid_nick(const std::string &nick)
{
	static const std::string special = "[]\\`_^{|}";
	if (nick.size() > 9)
		return false;
	
	if (!std::isalpha(nick[0]) && special.find(nick[0]) == std::string::npos)
		return false;
	
	for (int i = 0; i < nick.size(); i++) {
		char c = nick[i];
		if (!std::isalnum(c) && c != '-'
			&& special.find(nick[0]) == std::string::npos)
			return false;
	}
	return true;
}

/*
**	PASS <password>
**	- Must be sent before NICK/USER complete registration.
**	- No args:                       ERR_NEEDMOREPARAMS (461)
**	- Client already registered:     ERR_ALREADYREGISTRED (462)
**	- Password mismatches serv.getPassword(): record failure (the client
**	  will be rejected when NICK/USER tries to finalize registration), or
**	  reject immediately depending on the chosen registration policy.
**	- On success: mark the client as having supplied the correct password
**	  (extra Client flag may be needed).
*/


int	cmd_pass(Command& cmd, Server& serv) {
	if (cmd.getArguments().empty())
	{
		serv.sndmsg(cmd.getFd(), Reply::err_needmoreparams("", "", ""));
		return (1);
	}
	Clients&	clients = serv.getClients();
	Client* 	client = clients.get_client_fd(cmd.getFd());
	if  (!client)
	{
		Client newClient(cmd.getFd());
		if (clients.add_client(newClient))
			return (2);
		client = clients.get_client_fd(cmd.getFd());;
	}
	if (client->isRegistered())
	{
		serv.sndmsg(cmd.getFd(), Reply::err_alreadyregistred("", client->getNickname()));
		return (3);
	}
	if (cmd.getArguments()[0] == serv.getPassword())
	{
		serv.sndmsg(cmd.getFd(), Reply::err_passwdmismatch("", client->getNickname()));
    	return (4);
	}
	client->setAuthenticated(true);
	register_try(client, serv);
	return 0;
}

/*
**	NICK <nickname>
**	- No arg:                        ERR_NONICKNAMEGIVEN (431)
**	- Invalid chars / length > 9:    ERR_ERRONEUSNICKNAME (432)
**	- Nickname already in use
**	  (serv.getClients().get_client(nick) != NULL): ERR_NICKNAMEINUSE (433)
**	- On success:
**	    * update Client's nickname (Client::setNickname)
**	    * keep the Clients::lut (nick -> fd) in sync; consider adding a
**	      rename helper on Clients to do this atomically
**	    * if the client was already registered, broadcast
**	      ":<old>!<user>@<host> NICK :<new>" to every channel the client
**	      is in (iterate Client::getChannels()).
**	- If this is the first NICK and USER was already sent (or vice versa),
**	  finalize registration (welcome numerics 001-004).
*/
int	cmd_nick(Command& cmd, Server& serv) {
	Clients&	clients = serv.getClients();
	Client* 	client = clients.get_client_fd(cmd.getFd());

	if  (!client)
	{
		Client newClient(cmd.getFd());
		if (clients.add_client(newClient))
			return (1);
		client = clients.get_client_fd(cmd.getFd());;
	}

	std::string	oldnick = client->getNickname();
	std::string newnick;
	if (cmd.getArguments().empty())
	{
		serv.sndmsg(cmd.getFd(), Reply::err_nonicknamegiven("", oldnick));
		return (2);
	}
	newnick = cmd.getArguments()[0];
	if (!is_valid_nick(newnick)) {
		serv.sndmsg(cmd.getFd(), Reply::err_erroneusnickname("", oldnick, newnick));
		return (3);
	}
	if (clients.get_client(newnick)) {
		serv.sndmsg(cmd.getFd(), Reply::err_nicknameinuse("", oldnick, newnick));
		return (4);
	}
	clients.update_nick(cmd.getFd(), newnick);
	register_try(client, serv);
	return 0;
}

/*
**	USER <username> <hostname> <servername> :<realname>
**	- Fewer than 4 params (3 args + trailing): ERR_NEEDMOREPARAMS (461)
**	- Client already registered:               ERR_ALREADYREGISTRED (462)
**	- On success:
**	    * Client::setUsername / setRealname / setHostname / setServer
**	    * if NICK and PASS already done, finalize registration:
**	      send RPL_WELCOME (001), RPL_YOURHOST (002), RPL_CREATED (003),
**	      RPL_MYINFO (004), then mark Client::setRegistered(true).
*/
int	cmd_user(Command& cmd, Server& serv) {
	(void)cmd;
	(void)serv;
	return 0;
}

/*
**	QUIT [:<quit message>]
**	- Always succeeds; the trailing string is the quit reason
**	  (default to nickname or "Client Quit" if absent).
**	- For each channel the client is in: broadcast
**	  ":<nick>!<user>@<host> QUIT :<reason>" to other members, then
**	  remove the client from the channel (Channel::removeUser, removeOp).
**	  If the channel becomes empty, drop it from Channels.
**	- Send ERROR to the quitting client, then call
**	  serv.disconnectClient(cmd.getFd()) to close the socket and poll slot.
*/
int	cmd_quit(Command& cmd, Server& serv) {
	(void)cmd;
	(void)serv;
	return 0;
}
