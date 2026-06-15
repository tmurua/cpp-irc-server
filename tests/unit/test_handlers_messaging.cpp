#include "handler_helpers.hpp"

using namespace ht;

// ── PRIVMSG ───────────────────────────────────────────────────────────────────

TEST(handler_privmsg_no_args_sends_411)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "PRIVMSG");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "411"));
	close_peer(p);
}

TEST(handler_privmsg_no_text_sends_412)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "PRIVMSG bob");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "412"));
	close_peer(p);
}

TEST(handler_privmsg_unknown_nick_sends_401)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "PRIVMSG ghost :hello");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "401"));
	close_peer(p);
}

TEST(handler_privmsg_unknown_channel_sends_403)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "PRIVMSG #ghost :hi");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "403"));
	close_peer(p);
}

TEST(handler_privmsg_to_nick_delivered_to_target_only)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_registered(serv, alice.srv_fd, "alice");
	add_registered(serv, bob.srv_fd,   "bob");
	dispatch(serv, alice.srv_fd, "PRIVMSG bob :hello there");

	std::string bob_sees   = drain(bob.cap_fd);
	std::string alice_sees = drain(alice.cap_fd);
	ASSERT(has(bob_sees, "PRIVMSG"));
	ASSERT(has(bob_sees, "hello there"));
	ASSERT(alice_sees.empty());
	close_peer(alice);
	close_peer(bob);
}

TEST(handler_privmsg_to_channel_broadcasts_except_sender)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	Peer carol = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room");
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room");
	add_in_channel(serv, carol.srv_fd, "carol", "#room");

	dispatch(serv, alice.srv_fd, "PRIVMSG #room :group hi");
	std::string alice_sees = drain(alice.cap_fd);
	std::string bob_sees   = drain(bob.cap_fd);
	std::string carol_sees = drain(carol.cap_fd);

	ASSERT(alice_sees.empty());
	ASSERT(has(bob_sees,   "PRIVMSG"));
	ASSERT(has(bob_sees,   "group hi"));
	ASSERT(has(carol_sees, "PRIVMSG"));
	ASSERT(has(carol_sees, "group hi"));
	close_peer(alice);
	close_peer(bob);
	close_peer(carol);
}

TEST(handler_privmsg_multiple_targets_comma_split)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	Peer carol = make_peer();
	add_registered(serv, alice.srv_fd, "alice");
	add_registered(serv, bob.srv_fd,   "bob");
	add_registered(serv, carol.srv_fd, "carol");

	dispatch(serv, alice.srv_fd, "PRIVMSG bob,carol :hi both");
	ASSERT(has(drain(bob.cap_fd),   "hi both"));
	ASSERT(has(drain(carol.cap_fd), "hi both"));
	close_peer(alice);
	close_peer(bob);
	close_peer(carol);
}

// ── NOTICE ────────────────────────────────────────────────────────────────────

TEST(handler_notice_no_args_silent)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "NOTICE");
	ASSERT(drain(p.cap_fd).empty());
	close_peer(p);
}

TEST(handler_notice_no_text_silent)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "NOTICE bob");
	ASSERT(drain(p.cap_fd).empty());
	close_peer(p);
}

TEST(handler_notice_unknown_target_silent)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "NOTICE ghost :ping");
	std::string out = drain(p.cap_fd);
	ASSERT(out.empty());
	close_peer(p);
}

TEST(handler_notice_delivers_to_nick)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_registered(serv, alice.srv_fd, "alice");
	add_registered(serv, bob.srv_fd,   "bob");
	dispatch(serv, alice.srv_fd, "NOTICE bob :heads up");
	std::string bob_sees = drain(bob.cap_fd);
	ASSERT(has(bob_sees, "NOTICE"));
	ASSERT(has(bob_sees, "heads up"));
	close_peer(alice);
	close_peer(bob);
}

TEST(handler_notice_delivers_to_channel)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room");
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room");
	dispatch(serv, alice.srv_fd, "NOTICE #room :announcement");
	std::string bob_sees = drain(bob.cap_fd);
	ASSERT(has(bob_sees, "NOTICE"));
	ASSERT(has(bob_sees, "announcement"));
	close_peer(alice);
	close_peer(bob);
}

// ── PRIVMSG boundary cases ───────────────────────────────────────────────────

TEST(handler_privmsg_empty_tokens_in_comma_list_skipped)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	Peer carol = make_peer();
	add_registered(serv, alice.srv_fd, "alice");
	add_registered(serv, bob.srv_fd,   "bob");
	add_registered(serv, carol.srv_fd, "carol");
	dispatch(serv, alice.srv_fd, "PRIVMSG bob,,carol :hi both");
	ASSERT(has(drain(bob.cap_fd),   "hi both"));
	ASSERT(has(drain(carol.cap_fd), "hi both"));
	close_peer(alice);
	close_peer(bob);
	close_peer(carol);
}

TEST(handler_privmsg_to_channel_as_non_member_rejected_404)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_registered(serv, alice.srv_fd, "alice");
	add_in_channel(serv, bob.srv_fd, "bob", "#room");
	dispatch(serv, alice.srv_fd, "PRIVMSG #room :from outside");
	std::string alice_sees = drain(alice.cap_fd);
	std::string bob_sees   = drain(bob.cap_fd);
	ASSERT(has(alice_sees, "404"));
	ASSERT(bob_sees.empty());
	close_peer(alice);
	close_peer(bob);
}

TEST(handler_notice_to_channel_as_non_member_silent_and_not_delivered)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_registered(serv, alice.srv_fd, "alice");
	add_in_channel(serv, bob.srv_fd, "bob", "#room");
	dispatch(serv, alice.srv_fd, "NOTICE #room :from outside");
	ASSERT(drain(alice.cap_fd).empty());
	ASSERT(drain(bob.cap_fd).empty());
	close_peer(alice);
	close_peer(bob);
}
