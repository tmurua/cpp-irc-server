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

// ── Edge cases: empty / whitespace input ─────────────────────────────────────

TEST(command_empty_input_parses_as_unknown)
{
	Command	cmd = parse(1, "");

	ASSERT_EQ(cmd.getType(), CmdType::UNKNOWN);
	ASSERT(cmd.getPrefix().empty());
	ASSERT(cmd.getArguments().empty());
	ASSERT(cmd.getTrailing().empty());
}

TEST(command_crlf_only_parses_as_unknown)
{
	Command	cmd = parse(1, "\r\n");

	ASSERT_EQ(cmd.getType(), CmdType::UNKNOWN);
	ASSERT(cmd.getArguments().empty());
}

TEST(command_lf_only_parses_as_unknown)
{
	Command	cmd = parse(1, "\n");

	ASSERT_EQ(cmd.getType(), CmdType::UNKNOWN);
}

TEST(command_whitespace_only_parses_as_unknown)
{
	Command	cmd = parse(1, "   \r\n");

	ASSERT_EQ(cmd.getType(), CmdType::UNKNOWN);
	ASSERT(cmd.getArguments().empty());
}

// ── Edge cases: spacing / argument splitting ─────────────────────────────────

TEST(command_multiple_spaces_between_args_collapse)
{
	Command		cmd = parse(1, "PRIVMSG  bob  :hi\r\n");
	const char*	expected[] = { "bob" };

	ASSERT_EQ(cmd.getType(), CmdType::PRIVMSG);
	ASSERT(args_equal(cmd.getArguments(), expected, 1));
	ASSERT_EQ(cmd.getTrailing(), "hi");
}

TEST(command_no_args_no_trailing)
{
	Command	cmd = parse(1, "PONG\r\n");

	ASSERT_EQ(cmd.getType(), CmdType::PONG);
	ASSERT(cmd.getArguments().empty());
	ASSERT(cmd.getTrailing().empty());
}

TEST(command_many_args_no_trailing)
{
	Command		cmd = parse(1, "MODE #chan +o-v alice bob\r\n");
	const char*	expected[] = { "#chan", "+o-v", "alice", "bob" };

	ASSERT_EQ(cmd.getType(), CmdType::MODE);
	ASSERT(args_equal(cmd.getArguments(), expected, 4));
	ASSERT(cmd.getTrailing().empty());
}

// ── Edge cases: trailing parameter ────────────────────────────────────────────

TEST(command_trailing_only_colon_is_empty_trailing)
{
	Command	cmd = parse(1, "PING :\r\n");

	ASSERT_EQ(cmd.getType(), CmdType::PING);
	ASSERT(cmd.getArguments().empty());
	ASSERT(cmd.getTrailing().empty());
}

TEST(command_trailing_with_internal_colons_kept)
{
	Command	cmd = parse(1, "PRIVMSG bob :hello: world :again\r\n");

	ASSERT_EQ(cmd.getType(), CmdType::PRIVMSG);
	ASSERT_EQ(cmd.getTrailing(), "hello: world :again");
}

TEST(command_trailing_starts_with_colon)
{
	Command	cmd = parse(1, "PRIVMSG bob ::leading\r\n");

	ASSERT_EQ(cmd.getType(), CmdType::PRIVMSG);
	ASSERT_EQ(cmd.getTrailing(), ":leading");
}

TEST(command_trailing_only_no_args)
{
	Command	cmd = parse(1, "PING :tok en\r\n");

	ASSERT_EQ(cmd.getType(), CmdType::PING);
	ASSERT(cmd.getArguments().empty());
	ASSERT_EQ(cmd.getTrailing(), "tok en");
}

// ── Edge cases: command case-insensitivity ────────────────────────────────────

TEST(command_lowercase_recognized)
{
	Command	cmd = parse(1, "privmsg bob :hi\r\n");

	ASSERT_EQ(cmd.getType(), CmdType::PRIVMSG);
}

TEST(command_mixed_case_recognized)
{
	Command	cmd = parse(1, "PrIvMsG bob :hi\r\n");

	ASSERT_EQ(cmd.getType(), CmdType::PRIVMSG);
}

TEST(command_all_handlers_case_insensitive)
{
	ASSERT_EQ(parse(1, "pass secret\r\n").getType(),  CmdType::PASS);
	ASSERT_EQ(parse(1, "nick a\r\n").getType(),       CmdType::NICK);
	ASSERT_EQ(parse(1, "user u 0 * :r\r\n").getType(), CmdType::USER);
	ASSERT_EQ(parse(1, "quit\r\n").getType(),         CmdType::QUIT);
	ASSERT_EQ(parse(1, "join #c\r\n").getType(),      CmdType::JOIN);
	ASSERT_EQ(parse(1, "part #c\r\n").getType(),      CmdType::PART);
	ASSERT_EQ(parse(1, "topic #c\r\n").getType(),     CmdType::TOPIC);
	ASSERT_EQ(parse(1, "kick #c b\r\n").getType(),    CmdType::KICK);
	ASSERT_EQ(parse(1, "invite b #c\r\n").getType(),  CmdType::INVITE);
	ASSERT_EQ(parse(1, "mode #c\r\n").getType(),      CmdType::MODE);
	ASSERT_EQ(parse(1, "notice b :x\r\n").getType(),  CmdType::NOTICE);
	ASSERT_EQ(parse(1, "ping :x\r\n").getType(),      CmdType::PING);
	ASSERT_EQ(parse(1, "pong :x\r\n").getType(),      CmdType::PONG);
}

// ── Edge cases: prefix ────────────────────────────────────────────────────────

TEST(command_prefix_servername_only)
{
	Command	cmd = parse(1, ":irc.example.com PING :tok\r\n");

	ASSERT_EQ(cmd.getPrefix(), "irc.example.com");
	ASSERT_EQ(cmd.getType(), CmdType::PING);
	ASSERT_EQ(cmd.getTrailing(), "tok");
}

TEST(command_prefix_with_just_nick)
{
	Command	cmd = parse(1, ":alice JOIN #room\r\n");
	const char*	expected[] = { "#room" };

	ASSERT_EQ(cmd.getPrefix(), "alice");
	ASSERT_EQ(cmd.getType(), CmdType::JOIN);
	ASSERT(args_equal(cmd.getArguments(), expected, 1));
}

TEST(command_prefix_then_nothing_else_parses_as_unknown)
{
	// ":alice " has a space, so prefix gets extracted, leaving empty cmd → UNKNOWN
	Command	cmd = parse(1, ":alice \r\n");

	ASSERT_EQ(cmd.getPrefix(), "alice");
	ASSERT_EQ(cmd.getType(), CmdType::UNKNOWN);
}

TEST(command_prefix_no_space_throws)
{
	ASSERT_THROW(parse(1, ":noseparator\r\n"), Command::MalformedException);
}

// ── Edge cases: long line ─────────────────────────────────────────────────────

TEST(command_long_trailing_preserved)
{
	std::string	body(400, 'x');
	Command		cmd = parse(1, "PRIVMSG #c :" + body + "\r\n");

	ASSERT_EQ(cmd.getType(), CmdType::PRIVMSG);
	ASSERT_EQ(cmd.getTrailing(), body);
	ASSERT_EQ(cmd.getTrailing().size(), body.size());
}

// Bug fix: get_prefix must not dereference raw[0] when the line is empty after
// stripping CRLF (was UB before the empty-check was added).
TEST(command_prefix_check_does_not_crash_on_empty_line)
{
	// "\r\n" strips to "" — get_prefix must handle empty raw without UB.
	Command cmd = parse(1, "\r\n");
	ASSERT_EQ(cmd.getType(), CmdType::UNKNOWN);

	// Just "\n" strips to "" as well.
	Command cmd2 = parse(1, "\n");
	ASSERT_EQ(cmd2.getType(), CmdType::UNKNOWN);
}
