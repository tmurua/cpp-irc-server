#include "handler_helpers.hpp"

using namespace ht;

// ── PING ──────────────────────────────────────────────────────────────────────

TEST(handler_ping_no_args_sends_409)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "PING");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "409"));
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
