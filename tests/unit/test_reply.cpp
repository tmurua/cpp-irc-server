

#include "test_runner.hpp"
#include "Reply.hpp"
#include "Clients/Client.hpp"

static Client make_client(int fd, const char* nick, const char* user, const char* host)
{
	Client c(fd);
	c.setNickname(nick);
	c.setUsername(user);
	c.setHostname(host);
	return c;
}

// ── Primitives ────────────────────────────────────────────────────────────────

TEST(reply_from_server_with_trailing)
{
	std::string r = Reply::from_server("srv", "001", "alice", "Welcome!");
	ASSERT_EQ(r, ":srv 001 alice :Welcome!\r\n");
}

TEST(reply_from_server_no_trailing)
{
	std::string r = Reply::from_server("srv", "004", "alice param1");
	ASSERT_EQ(r, ":srv 004 alice param1\r\n");
}

TEST(reply_from_client_params_only)
{
	Client c = make_client(1, "alice", "alice", "localhost");
	std::string r = Reply::from_client(c, "JOIN", "#test");
	ASSERT_EQ(r, ":alice!alice@localhost JOIN #test\r\n");
}

TEST(reply_from_client_with_trailing)
{
	Client c = make_client(1, "alice", "alice", "localhost");
	std::string r = Reply::from_client(c, "PRIVMSG", "#test", "hello");
	ASSERT_EQ(r, ":alice!alice@localhost PRIVMSG #test :hello\r\n");
}

TEST(reply_from_client_no_params_no_trailing)
{
	Client c = make_client(1, "alice", "alice", "localhost");
	std::string r = Reply::from_client(c, "PONG");
	ASSERT_EQ(r, ":alice!alice@localhost PONG\r\n");
}

// ── Registration ──────────────────────────────────────────────────────────────

TEST(reply_rpl_welcome_contains_001_and_nick)
{
	Client c = make_client(1, "alice", "alice", "localhost");
	std::string r = Reply::rpl_welcome("srv", c);
	ASSERT(r.find("001") != std::string::npos);
	ASSERT(r.find("alice") != std::string::npos);
	ASSERT(r.size() >= 2 && r[r.size()-2] == '\r' && r[r.size()-1] == '\n');
}

TEST(reply_rpl_yourhost_contains_002)
{
	Client c = make_client(1, "alice", "alice", "localhost");
	std::string r = Reply::rpl_yourhost("srv", c);
	ASSERT(r.find("002") != std::string::npos);
}

TEST(reply_rpl_created_contains_003)
{
	Client c = make_client(1, "alice", "alice", "localhost");
	ASSERT(Reply::rpl_created("srv", c).find("003") != std::string::npos);
}

TEST(reply_rpl_myinfo_contains_004)
{
	Client c = make_client(1, "alice", "alice", "localhost");
	ASSERT(Reply::rpl_myinfo("srv", c).find("004") != std::string::npos);
}

// ── Channel numerics ──────────────────────────────────────────────────────────

TEST(reply_rpl_topic_contains_332_and_topic)
{
	std::string r = Reply::rpl_topic("srv", "alice", "#chan", "My Topic");
	ASSERT(r.find("332") != std::string::npos);
	ASSERT(r.find("My Topic") != std::string::npos);
}

TEST(reply_rpl_notopic_contains_331)
{
	ASSERT(Reply::rpl_notopic("srv", "alice", "#chan").find("331") != std::string::npos);
}

TEST(reply_rpl_namreply_contains_353_and_names)
{
	std::string r = Reply::rpl_namreply("srv", "alice", "#chan", "@alice bob");
	ASSERT(r.find("353") != std::string::npos);
	ASSERT(r.find("@alice") != std::string::npos);
}

TEST(reply_rpl_endofnames_contains_366)
{
	ASSERT(Reply::rpl_endofnames("srv", "alice", "#chan").find("366") != std::string::npos);
}

TEST(reply_rpl_inviting_contains_341)
{
	ASSERT(Reply::rpl_inviting("srv", "alice", "bob", "#chan").find("341") != std::string::npos);
}

TEST(reply_rpl_channelmodeis_contains_324)
{
	ASSERT(Reply::rpl_channelmodeis("srv", "alice", "#chan", "+it").find("324") != std::string::npos);
}

// ── Error numerics ────────────────────────────────────────────────────────────

TEST(reply_err_nosuchnick_contains_401)
{
	ASSERT(Reply::err_nosuchnick("srv", "alice", "ghost").find("401") != std::string::npos);
}

TEST(reply_err_nosuchchannel_contains_403)
{
	ASSERT(Reply::err_nosuchchannel("srv", "alice", "#none").find("403") != std::string::npos);
}

TEST(reply_err_needmoreparams_contains_461_and_cmd)
{
	std::string r = Reply::err_needmoreparams("srv", "alice", "PASS");
	ASSERT(r.find("461") != std::string::npos);
	ASSERT(r.find("PASS") != std::string::npos);
}

TEST(reply_err_nicknameinuse_contains_433_and_nick)
{
	std::string r = Reply::err_nicknameinuse("srv", "*", "alice");
	ASSERT(r.find("433") != std::string::npos);
	ASSERT(r.find("alice") != std::string::npos);
}

TEST(reply_err_alreadyregistred_contains_462)
{
	ASSERT(Reply::err_alreadyregistred("srv", "alice").find("462") != std::string::npos);
}

TEST(reply_err_passwdmismatch_contains_464)
{
	ASSERT(Reply::err_passwdmismatch("srv", "alice").find("464") != std::string::npos);
}

TEST(reply_err_chanoprivsneeded_contains_482)
{
	ASSERT(Reply::err_chanoprivsneeded("srv", "alice", "#chan").find("482") != std::string::npos);
}

TEST(reply_err_usernotinchannel_contains_441)
{
	ASSERT(Reply::err_usernotinchannel("srv", "alice", "bob", "#chan").find("441") != std::string::npos);
}

TEST(reply_err_unknownmode_contains_472_and_char)
{
	std::string r = Reply::err_unknownmode("srv", "alice", 'z');
	ASSERT(r.find("472") != std::string::npos);
	ASSERT(r.find("z") != std::string::npos);
}

TEST(reply_all_terminate_with_crlf)
{
	std::vector<std::string> replies;
	Client c = make_client(1, "alice", "alice", "localhost");
	replies.push_back(Reply::rpl_welcome("srv", c));
	replies.push_back(Reply::err_nosuchnick("srv", "alice", "x"));
	replies.push_back(Reply::err_nosuchchannel("srv", "alice", "#x"));
	replies.push_back(Reply::err_needmoreparams("srv", "alice", "X"));
	replies.push_back(Reply::err_chanoprivsneeded("srv", "alice", "#x"));
	replies.push_back(Reply::rpl_endofnames("srv", "alice", "#x"));
	for (std::size_t i = 0; i < replies.size(); i++) {
		const std::string& r = replies[i];
		ASSERT(r.size() >= 2);
		ASSERT(r[r.size()-2] == '\r' && r[r.size()-1] == '\n');
	}
}
