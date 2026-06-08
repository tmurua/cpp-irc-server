/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_cmdtypes.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/08 12:00:00 by johartma          #+#    #+#             */
/*   Updated: 2026/06/08 12:00:00 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test_runner.hpp"
#include "Command/CmdTypes.hpp"

struct CmdCase {
	const char*		input;
	CmdType::Type	expected;
};

static const CmdCase	g_known_cmds[] = {
	{ "PASS",    CmdType::PASS },
	{ "NICK",    CmdType::NICK },
	{ "USER",    CmdType::USER },
	{ "OPER",    CmdType::OPER },
	{ "MODE",    CmdType::MODE },
	{ "SERVICE", CmdType::SERVICE },
	{ "QUIT",    CmdType::QUIT },
	{ "SQUIT",   CmdType::SQUIT },
	{ "JOIN",    CmdType::JOIN },
	{ "PART",    CmdType::PART },
	{ "TOPIC",   CmdType::TOPIC },
	{ "NAMES",   CmdType::NAMES },
	{ "LIST",    CmdType::LIST },
	{ "INVITE",  CmdType::INVITE },
	{ "KICK",    CmdType::KICK },
	{ "PRIVMSG", CmdType::PRIVMSG },
	{ "NOTICE",  CmdType::NOTICE },
	{ "MOTD",    CmdType::MOTD },
	{ "LUSERS",  CmdType::LUSERS },
	{ "VERSION", CmdType::VERSION },
	{ "STATS",   CmdType::STATS },
	{ "LINKS",   CmdType::LINKS },
	{ "TIME",    CmdType::TIME },
	{ "CONNECT", CmdType::CONNECT },
	{ "TRACE",   CmdType::TRACE },
	{ "ADMIN",   CmdType::ADMIN },
	{ "INFO",    CmdType::INFO },
	{ "SERVLIST", CmdType::SERVLIST },
	{ "SQUERY",  CmdType::SQUERY },
	{ "WHO",     CmdType::WHO },
	{ "WHOIS",   CmdType::WHOIS },
	{ "WHOWAS",  CmdType::WHOWAS },
	{ "KILL",    CmdType::KILL },
	{ "PING",    CmdType::PING },
	{ "PONG",    CmdType::PONG },
	{ "ERROR",   CmdType::ERROR },
};

TEST(parse_cmdtype_ping_uppercase)
{
	ASSERT_EQ(parse_cmdtype("PING"), CmdType::PING);
}

TEST(parse_cmdtype_ping_lowercase)
{
	ASSERT_EQ(parse_cmdtype("ping"), CmdType::PING);
}

TEST(parse_cmdtype_ping_mixedcase)
{
	ASSERT_EQ(parse_cmdtype("PiNg"), CmdType::PING);
}

TEST(parse_cmdtype_representative_commands)
{
	ASSERT_EQ(parse_cmdtype("NICK"), CmdType::NICK);
	ASSERT_EQ(parse_cmdtype("PRIVMSG"), CmdType::PRIVMSG);
	ASSERT_EQ(parse_cmdtype("JOIN"), CmdType::JOIN);
	ASSERT_EQ(parse_cmdtype("QUIT"), CmdType::QUIT);
}

TEST(parse_cmdtype_unknown_commands)
{
	ASSERT_EQ(parse_cmdtype("FOOBAR"), CmdType::UNKNOWN);
	ASSERT_EQ(parse_cmdtype(""), CmdType::UNKNOWN);
	ASSERT_EQ(parse_cmdtype("123"), CmdType::UNKNOWN);
}

TEST(parse_cmdtype_all_table_entries)
{
	for (size_t i = 0; i < sizeof(g_known_cmds) / sizeof(g_known_cmds[0]); ++i) {
		ASSERT_EQ(parse_cmdtype(g_known_cmds[i].input),
				  g_known_cmds[i].expected);
	}
}
