#include "handler_helpers.hpp"

using namespace ht;

// ── PASS ──────────────────────────────────────────────────────────────────────

TEST(handler_pass_no_args_sends_461)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "PASS");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "461"));
	ASSERT(has(out, "PASS"));
	close_peer(p);
}

TEST(handler_pass_wrong_password_sends_464)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "PASS wrongpass");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "464"));
	close_peer(p);
}

TEST(handler_pass_correct_password_authenticates)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "PASS secret");
	std::string out = drain(p.cap_fd);
	ASSERT(out.empty());
	Client* c = serv.getClients().get_client_fd(p.srv_fd);
	ASSERT(c != NULL);
	ASSERT(c->isAuthenticated());
	close_peer(p);
}

TEST(handler_pass_when_already_registered_sends_462)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "PASS secret");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "462"));
	close_peer(p);
}

// ── NICK ──────────────────────────────────────────────────────────────────────

TEST(handler_nick_no_args_sends_431)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "NICK");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "431"));
	close_peer(p);
}

TEST(handler_nick_invalid_chars_sends_432)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "NICK 1bad");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "432"));
	close_peer(p);
}

TEST(handler_nick_too_long_sends_432)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "NICK abcdefghij");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "432"));
	close_peer(p);
}

TEST(handler_nick_in_use_sends_433)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_registered(serv, alice.srv_fd, "alice");
	dispatch(serv, bob.srv_fd, "NICK alice");
	std::string out = drain(bob.cap_fd);
	ASSERT(has(out, "433"));
	close_peer(alice);
	close_peer(bob);
}

TEST(handler_nick_valid_updates_nickname)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "NICK alice");
	Client* c = serv.getClients().get_client_fd(p.srv_fd);
	ASSERT(c != NULL);
	ASSERT_EQ(c->getNickname(), std::string("alice"));
	close_peer(p);
}

TEST(handler_nick_change_while_registered_broadcasts_to_channel)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room", true);
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room");
	(void)drain(alice.cap_fd);
	(void)drain(bob.cap_fd);

	dispatch(serv, alice.srv_fd, "NICK newalice");
	std::string seen_by_bob = drain(bob.cap_fd);
	ASSERT(has(seen_by_bob, "NICK"));
	ASSERT(has(seen_by_bob, "newalice"));

	Channel* chan = serv.getChannels().get_channel("#room");
	ASSERT(chan != NULL);
	ASSERT(chan->hasUser("newalice"));
	ASSERT(!chan->hasUser("alice"));
	ASSERT(chan->isOp("newalice"));
	close_peer(alice);
	close_peer(bob);
}

// ── USER ──────────────────────────────────────────────────────────────────────

TEST(handler_user_too_few_args_sends_461)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "USER alice 0");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "461"));
	close_peer(p);
}

TEST(handler_user_empty_trailing_sends_461)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "USER alice 0 *");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "461"));
	close_peer(p);
}

TEST(handler_user_when_registered_sends_462)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "USER alice 0 * :Alice");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "462"));
	close_peer(p);
}

TEST(handler_user_valid_sets_fields)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "USER bob host server :Bob Real");
	Client* c = serv.getClients().get_client_fd(p.srv_fd);
	ASSERT(c != NULL);
	ASSERT_EQ(c->getUsername(), std::string("bob"));
	// Per RFC 2812 the hostname/servername fields from USER are ignored.
	ASSERT(c->getHostname() != "host");
	ASSERT(c->getServer()   != "server");
	ASSERT_EQ(c->getRealname(), std::string("Bob Real"));
	close_peer(p);
}

TEST(handler_full_registration_sequence_sends_welcome_burst)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "PASS secret");
	dispatch(serv, p.srv_fd, "NICK alice");
	dispatch(serv, p.srv_fd, "USER alice 0 * :Alice");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "001"));
	ASSERT(has(out, "ft_irc.local 001"));
	ASSERT(has(out, "002"));
	ASSERT(has(out, "003"));
	ASSERT(has(out, "004"));
	Client* c = serv.getClients().get_client_fd(p.srv_fd);
	ASSERT(c != NULL);
	ASSERT(c->isRegistered());
	close_peer(p);
}

// ── QUIT ──────────────────────────────────────────────────────────────────────

TEST(handler_quit_removes_client_from_container)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "QUIT :bye");
	ASSERT(serv.getClients().get_client_fd(p.srv_fd) == NULL);
	close_peer(p);
}

TEST(handler_quit_broadcasts_to_channel_members)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room");
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room");
	(void)drain(bob.cap_fd);

	dispatch(serv, alice.srv_fd, "QUIT :leaving");
	std::string seen_by_bob = drain(bob.cap_fd);
	ASSERT(has(seen_by_bob, "QUIT"));
	ASSERT(has(seen_by_bob, "leaving"));
	close_peer(alice);
	close_peer(bob);
}

TEST(handler_quit_drops_empty_channel)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room", true);
	dispatch(serv, p.srv_fd, "QUIT :bye");
	ASSERT(serv.getChannels().get_channel("#room") == NULL);
	close_peer(p);
}

TEST(handler_quit_keeps_nonempty_channel)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room", true);
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room");
	dispatch(serv, alice.srv_fd, "QUIT");
	Channel* chan = serv.getChannels().get_channel("#room");
	ASSERT(chan != NULL);
	ASSERT(!chan->hasUser("alice"));
	ASSERT(chan->hasUser("bob"));
	close_peer(alice);
	close_peer(bob);
}

// ── NICK boundary cases ──────────────────────────────────────────────────────

TEST(handler_nick_with_special_chars_accepted)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "NICK [bot]");
	Client* c = serv.getClients().get_client_fd(p.srv_fd);
	ASSERT(c != NULL);
	ASSERT_EQ(c->getNickname(), std::string("[bot]"));
	close_peer(p);
}

TEST(handler_nick_exactly_9_chars_accepted)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "NICK abcdefghi");
	Client* c = serv.getClients().get_client_fd(p.srv_fd);
	ASSERT(c != NULL);
	ASSERT_EQ(c->getNickname(), std::string("abcdefghi"));
	close_peer(p);
}

TEST(handler_nick_starts_with_hyphen_rejected)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "NICK -name");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "432"));
	close_peer(p);
}

TEST(handler_nick_with_hyphen_in_body_accepted)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "NICK alice-1");
	Client* c = serv.getClients().get_client_fd(p.srv_fd);
	ASSERT(c != NULL);
	ASSERT_EQ(c->getNickname(), std::string("alice-1"));
	close_peer(p);
}

// Bug: NICK to own current nickname must be a silent no-op, not ERR_NICKNAMEINUSE (433).
TEST(handler_nick_same_name_is_noop)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	(void)drain(p.cap_fd);
	dispatch(serv, p.srv_fd, "NICK alice");
	std::string out = drain(p.cap_fd);
	ASSERT(!has(out, "433"));
	Client* c = serv.getClients().get_client_fd(p.srv_fd);
	ASSERT(c != NULL);
	ASSERT_EQ(c->getNickname(), std::string("alice"));
	close_peer(p);
}

// Bug fix: NICK relay was sent to the renaming client once per shared channel
// (i.e. once per channel they were in), instead of exactly once.
TEST(handler_nick_self_relay_sent_exactly_once_when_in_multiple_channels)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	// Put alice in two channels so the per-channel loop runs twice.
	add_in_channel(serv, alice.srv_fd, "alice", "#room1", true);
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room1");
	add_in_channel(serv, alice.srv_fd, "alice", "#room2", true);
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room2");
	(void)drain(alice.cap_fd);
	(void)drain(bob.cap_fd);

	dispatch(serv, alice.srv_fd, "NICK newalice");

	std::string alice_sees = drain(alice.cap_fd);
	// Count NICK lines received by alice — must be exactly one.
	// The relay format is ":oldnick!... NICK :newalice\r\n" (trailing form).
	std::size_t count = 0;
	std::size_t pos = 0;
	while ((pos = alice_sees.find("NICK :newalice", pos)) != std::string::npos) {
		++count;
		pos += 1;
	}
	ASSERT_EQ(count, (std::size_t)1);
	close_peer(alice);
	close_peer(bob);
}
