/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_command.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/08 12:00:00 by johartma          #+#    #+#             */
/*   Updated: 2026/06/08 12:00:00 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test_runner.hpp"
#include "Command/Command.hpp"

static Command	parse(int fd, const std::string& raw)
{
	return Command(fd, raw);
}

static bool	args_equal(const std::vector<std::string>& args,
					   const char* expected[], size_t count)
{
	if (args.size() != count)
		return false;
	for (size_t i = 0; i < count; ++i) {
		if (args[i] != expected[i])
			return false;
	}
	return true;
}

TEST(command_ping_crlf)
{
	Command	cmd = parse(3, "PING\r\n");

	ASSERT_EQ(cmd.getFd(), 3);
	ASSERT_EQ(cmd.getType(), CmdType::PING);
	ASSERT(cmd.getPrefix().empty());
	ASSERT(cmd.getArguments().empty());
	ASSERT(cmd.getTrailing().empty());
}

TEST(command_ping_lf_only)
{
	Command	cmd = parse(3, "PING\n");

	ASSERT_EQ(cmd.getType(), CmdType::PING);
	ASSERT(cmd.getPrefix().empty());
	ASSERT(cmd.getArguments().empty());
	ASSERT(cmd.getTrailing().empty());
}

TEST(command_nick)
{
	Command			cmd = parse(1, "NICK mynick\r\n");
	const char*		expected[] = { "mynick" };

	ASSERT_EQ(cmd.getType(), CmdType::NICK);
	ASSERT(args_equal(cmd.getArguments(), expected, 1));
	ASSERT(cmd.getTrailing().empty());
}

TEST(command_privmsg_with_trailing)
{
	Command			cmd = parse(1, "PRIVMSG #chan :hello world\r\n");
	const char*		expected[] = { "#chan" };

	ASSERT_EQ(cmd.getType(), CmdType::PRIVMSG);
	ASSERT(args_equal(cmd.getArguments(), expected, 1));
	ASSERT_EQ(cmd.getTrailing(), "hello world");
}

TEST(command_privmsg_empty_trailing)
{
	Command			cmd = parse(1, "PRIVMSG #chan :\r\n");
	const char*		expected[] = { "#chan" };

	ASSERT_EQ(cmd.getType(), CmdType::PRIVMSG);
	ASSERT(args_equal(cmd.getArguments(), expected, 1));
	ASSERT(cmd.getTrailing().empty());
}

TEST(command_with_prefix)
{
	Command			cmd = parse(1, ":nick!u@h PRIVMSG #c :hi\r\n");
	const char*		expected[] = { "#c" };

	ASSERT_EQ(cmd.getPrefix(), "nick!u@h");
	ASSERT_EQ(cmd.getType(), CmdType::PRIVMSG);
	ASSERT(args_equal(cmd.getArguments(), expected, 1));
	ASSERT_EQ(cmd.getTrailing(), "hi");
}

TEST(command_mode_multiple_args)
{
	Command			cmd = parse(1, "MODE #chan +o nick");
	const char*		expected[] = { "#chan", "+o", "nick" };

	ASSERT_EQ(cmd.getType(), CmdType::MODE);
	ASSERT(args_equal(cmd.getArguments(), expected, 3));
	ASSERT(cmd.getTrailing().empty());
}

TEST(command_malformed_prefix)
{
	ASSERT_THROW(parse(1, ":onlyprefix\r\n"), Command::MalformedException);
}

TEST(command_unknown_type)
{
	Command			cmd = parse(1, "UNKNOWNCMD foo\r\n");
	const char*		expected[] = { "foo" };

	ASSERT_EQ(cmd.getType(), CmdType::UNKNOWN);
	ASSERT(args_equal(cmd.getArguments(), expected, 1));
}

TEST(command_fd_preserved)
{
	Command	cmd = parse(42, "PING\r\n");

	ASSERT_EQ(cmd.getFd(), 42);
}
