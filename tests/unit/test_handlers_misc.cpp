#include "handler_helpers.hpp"

using namespace ht;

// ── PING ──────────────────────────────────────────────────────────────────────

TEST(handler_ping_no_args_sends_pong)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "PING");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "PONG"));
	ASSERT(has(out, "ft_irc.local"));
	close_peer(p);
}

TEST(handler_ping_with_token_sends_pong)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "PING :sometoken");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "PONG"));
	ASSERT(has(out, "sometoken"));
	close_peer(p);
}

TEST(handler_ping_with_arg_sends_pong)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "PING server.example");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "PONG"));
	ASSERT(has(out, "server.example"));
	close_peer(p);
}

// ── PONG ──────────────────────────────────────────────────────────────────────

TEST(handler_pong_is_noop)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "PONG :anything");
	ASSERT(drain(p.cap_fd).empty());
	close_peer(p);
}

// ── UNKNOWN ───────────────────────────────────────────────────────────────────

TEST(handler_unknown_command_sends_421)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "WHOIS alice");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "421"));
	close_peer(p);
}

TEST(handler_unknown_command_silent_when_no_client)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	// No client added — cmd_unknown should not crash and not send anything
	dispatch(serv, p.srv_fd, "WHOIS alice");
	ASSERT(drain(p.cap_fd).empty());
	close_peer(p);
}

// ── CAP ───────────────────────────────────────────────────────────────────────

TEST(handler_cap_ls_before_registration)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "CAP LS 302");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "CAP * LS"));
	close_peer(p);
}

TEST(handler_cap_end_is_silent)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "CAP END");
	ASSERT(drain(p.cap_fd).empty());
	close_peer(p);
}

TEST(handler_cap_req_sends_nak)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "CAP REQ :multi-prefix");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "CAP * NAK"));
	close_peer(p);
}

TEST(handler_who_channel_lists_members)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#lobby", true);
	add_in_channel(serv, bob.srv_fd,   "bob",   "#lobby");
	dispatch(serv, alice.srv_fd, "WHO #lobby");
	std::string out = drain(alice.cap_fd);
	ASSERT(has(out, "352"));
	ASSERT(has(out, "bob"));
	ASSERT(has(out, "315"));
	ASSERT(has(out, "#lobby"));
	close_peer(alice);
	close_peer(bob);
}

TEST(handler_mode_ban_list_query_returns_368)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#lobby");
	dispatch(serv, p.srv_fd, "MODE #lobby b");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "368"));
	ASSERT(!has(out, "482"));
	close_peer(p);
}
