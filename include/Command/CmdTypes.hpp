/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CmdTypes.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 18:00:50 by johartma          #+#    #+#             */
/*   Updated: 2026/06/15 18:14:02 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CMDTYPES_HPP
# define CMDTYPES_HPP
# include <string>

namespace CmdType {
    enum Type {
        PASS, NICK, USER, CAP, OPER, MODE, SERVICE, QUIT, SQUIT, //Auth
        JOIN, PART, TOPIC, NAMES, LIST, INVITE, KICK, // Channels
        PRIVMSG, NOTICE, // Messages
        MOTD, LUSERS, VERSION, STATS, LINKS,
        TIME, CONNECT, TRACE, ADMIN, INFO, // Server Queries + Cmds
        SERVLIST, SQUERY, //Services
        WHO, WHOIS, WHOWAS, // User info
        KILL, PING, PONG, ERROR, // Misc
		UNKNOWN
    };
};

CmdType::Type   parse_cmdtype(const std::string& s);

#endif