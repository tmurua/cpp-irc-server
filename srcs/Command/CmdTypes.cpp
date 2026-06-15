/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CmdTypes.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/08 10:17:44 by johartma          #+#    #+#             */
/*   Updated: 2026/06/15 18:16:19 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command/CmdTypes.hpp"
#include <map>
#include <string>
#include <algorithm>
#include <cctype>

static char safe_toupper(char c) {
    return static_cast<char>(::toupper(static_cast<unsigned char>(c)));
}

CmdType::Type parse_cmdtype(const std::string& s) {
    typedef std::map<std::string, CmdType::Type> CmdMap;
    static CmdMap table;
    
	if (table.empty()) {
        // Auth
        table["PASS"]    = CmdType::PASS;
        table["NICK"]    = CmdType::NICK;
        table["USER"]    = CmdType::USER;
        table["OPER"]    = CmdType::OPER;
        table["MODE"]    = CmdType::MODE;
        table["SERVICE"] = CmdType::SERVICE;
        table["QUIT"]    = CmdType::QUIT;
        table["SQUIT"]   = CmdType::SQUIT;
        // Channels
        table["JOIN"]    = CmdType::JOIN;
        table["PART"]    = CmdType::PART;
        table["TOPIC"]   = CmdType::TOPIC;
        table["NAMES"]   = CmdType::NAMES;
        table["LIST"]    = CmdType::LIST;
        table["INVITE"]  = CmdType::INVITE;
        table["KICK"]    = CmdType::KICK;
        // Messages
        table["PRIVMSG"] = CmdType::PRIVMSG;
        table["NOTICE"]  = CmdType::NOTICE;
        // Server queries + cmds
        table["MOTD"]    = CmdType::MOTD;
        table["LUSERS"]  = CmdType::LUSERS;
        table["VERSION"] = CmdType::VERSION;
        table["STATS"]   = CmdType::STATS;
        table["LINKS"]   = CmdType::LINKS;
        table["TIME"]    = CmdType::TIME;
        table["CONNECT"] = CmdType::CONNECT;
        table["TRACE"]   = CmdType::TRACE;
        table["ADMIN"]   = CmdType::ADMIN;
        table["INFO"]    = CmdType::INFO;
        // Services
        table["SERVLIST"] = CmdType::SERVLIST;
        table["SQUERY"]   = CmdType::SQUERY;
        // User info
        table["WHO"]     = CmdType::WHO;
        table["WHOIS"]   = CmdType::WHOIS;
        table["WHOWAS"]  = CmdType::WHOWAS;
        // Misc
        table["KILL"]    = CmdType::KILL;
        table["PING"]    = CmdType::PING;
        table["PONG"]    = CmdType::PONG;
        table["ERROR"]   = CmdType::ERROR;
    }
    
	std::string t = s;
	std::transform(t.begin(), t.end(), t.begin(), safe_toupper);

    CmdMap::const_iterator it = table.find(t);
    return it != table.end() ? it->second : CmdType::UNKNOWN;
}
