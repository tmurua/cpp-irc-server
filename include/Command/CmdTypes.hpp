/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CmdTypes.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 18:00:50 by johartma          #+#    #+#             */
/*   Updated: 2026/06/05 18:45:13 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CMDTYPES_HPP
# define CMDTYPES_HPP
# include <string>

namespace CmdType {
    enum Type {
        PASS, NICK, USER, OPER, MODE, SERVICE, QUIT, SQUIT, //Auth
        JOIN, PART, TOPIC, NAMES, LIST, INVITE, KICK, // Channels
        PRIVMSG, NOTICE, // Messages
        MOTD, LUSERS, VERSION, STATS, LINKS,
        TIME, CONNECT, TRACE, ADMIN, INFO, // Server Queries + Cmds
        SERVLIST, SQUERY, //Services
        WHO, WHOIS, WHOWAS, // User info
        KILL, PING, PONG, ERROR // Misc
    };
};

CmdType::Type   parse_cmdtype(const std::string& s);

#endif