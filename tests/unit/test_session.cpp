#include "handler_helpers.hpp"

using namespace ht;

// End-to-end IRC session driven through Server::executecmd. Two clients run
// the full PASS → NICK → USER → JOIN → PRIVMSG → PART → QUIT lifecycle and
// each transition is verified at the wire level via socketpair capture.
TEST(session_two_client_full_lifecycle)
{
	Server serv("6667", "secret");
	Peer   alice = make_peer();
	Peer   bob   = make_peer();

	// 1. Alice registers
	dispatch(serv, alice.srv_fd, "PASS secret");
	dispatch(serv, alice.srv_fd, "NICK alice");
	dispatch(serv, alice.srv_fd, "USER alice 0 * :Alice");
	{
		std::string out = drain(alice.cap_fd);
		ASSERT(has(out, "001"));
		ASSERT(has(out, "002"));
		ASSERT(has(out, "003"));
		ASSERT(has(out, "004"));
	}
	{
		Client* c = serv.getClients().get_client_fd(alice.srv_fd);
		ASSERT(c != NULL);
		ASSERT(c->isRegistered());
	}

	// 2. Alice creates #room (becomes operator)
	dispatch(serv, alice.srv_fd, "JOIN #room");
	{
		std::string out = drain(alice.cap_fd);
		ASSERT(has(out, "JOIN"));
		ASSERT(has(out, "#room"));
		ASSERT(has(out, "353"));
		ASSERT(has(out, "366"));
		Channel* chan = serv.getChannels().get_channel("#room");
		ASSERT(chan != NULL);
		ASSERT(chan->isOp("alice"));
	}

	// 3. Bob registers
	dispatch(serv, bob.srv_fd, "PASS secret");
	dispatch(serv, bob.srv_fd, "NICK bob");
	dispatch(serv, bob.srv_fd, "USER bob 0 * :Bob");
	{
		std::string out = drain(bob.cap_fd);
		ASSERT(has(out, "001"));
		ASSERT(has(out, "004"));
	}

	// 4. Bob joins #room → Alice sees the JOIN relay
	dispatch(serv, bob.srv_fd, "JOIN #room");
	{
		std::string alice_sees = drain(alice.cap_fd);
		std::string bob_sees   = drain(bob.cap_fd);
		ASSERT(has(alice_sees, "JOIN"));
		ASSERT(has(alice_sees, "bob"));
		ASSERT(has(bob_sees, "353"));
		Channel* chan = serv.getChannels().get_channel("#room");
		ASSERT(chan->hasUser("alice"));
		ASSERT(chan->hasUser("bob"));
		ASSERT(!chan->isOp("bob"));
	}

	// 5. Alice → channel: Bob receives, Alice does not
	dispatch(serv, alice.srv_fd, "PRIVMSG #room :morning everyone");
	{
		std::string alice_sees = drain(alice.cap_fd);
		std::string bob_sees   = drain(bob.cap_fd);
		ASSERT(alice_sees.empty());
		ASSERT(has(bob_sees, "PRIVMSG"));
		ASSERT(has(bob_sees, "morning everyone"));
	}

	// 6. Bob → alice direct
	dispatch(serv, bob.srv_fd, "PRIVMSG alice :hi alice");
	{
		std::string alice_sees = drain(alice.cap_fd);
		ASSERT(has(alice_sees, "PRIVMSG"));
		ASSERT(has(alice_sees, "hi alice"));
		ASSERT(has(alice_sees, "bob"));
	}

	// 7. Alice promotes Bob to op via MODE +o
	dispatch(serv, alice.srv_fd, "MODE #room +o bob");
	{
		Channel* chan = serv.getChannels().get_channel("#room");
		ASSERT(chan->isOp("bob"));
		std::string bob_sees = drain(bob.cap_fd);
		ASSERT(has(bob_sees, "MODE"));
		ASSERT(has(bob_sees, "+o"));
		(void)drain(alice.cap_fd);
	}

	// 8. Bob changes the topic (still allowed without +t)
	dispatch(serv, bob.srv_fd, "TOPIC #room :daily standup");
	{
		Channel* chan = serv.getChannels().get_channel("#room");
		ASSERT_EQ(chan->getTopic(), std::string("daily standup"));
		std::string alice_sees = drain(alice.cap_fd);
		ASSERT(has(alice_sees, "TOPIC"));
		ASSERT(has(alice_sees, "daily standup"));
	}

	// 9. Alice parts
	dispatch(serv, alice.srv_fd, "PART #room :brb");
	{
		std::string bob_sees = drain(bob.cap_fd);
		ASSERT(has(bob_sees, "PART"));
		ASSERT(has(bob_sees, "brb"));
		Channel* chan = serv.getChannels().get_channel("#room");
		ASSERT(chan != NULL);
		ASSERT(!chan->hasUser("alice"));
		ASSERT(chan->hasUser("bob"));
	}

	// 10. Bob quits — channel becomes empty and is dropped
	dispatch(serv, bob.srv_fd, "QUIT :gtg");
	{
		ASSERT(serv.getClients().get_client_fd(bob.srv_fd) == NULL);
		ASSERT(serv.getChannels().get_channel("#room") == NULL);
	}

	close_peer(alice);
	close_peer(bob);
}

// Bob changes nick mid-session: the rename must propagate to channel state
// AND every co-located client must see the NICK relay.
TEST(session_nick_change_mid_session_propagates)
{
	Server serv("6667", "secret");
	Peer   alice = make_peer();
	Peer   bob   = make_peer();

	dispatch(serv, alice.srv_fd, "PASS secret");
	dispatch(serv, alice.srv_fd, "NICK alice");
	dispatch(serv, alice.srv_fd, "USER alice 0 * :Alice");
	dispatch(serv, alice.srv_fd, "JOIN #room");

	dispatch(serv, bob.srv_fd, "PASS secret");
	dispatch(serv, bob.srv_fd, "NICK bob");
	dispatch(serv, bob.srv_fd, "USER bob 0 * :Bob");
	dispatch(serv, bob.srv_fd, "JOIN #room");

	(void)drain(alice.cap_fd);
	(void)drain(bob.cap_fd);

	dispatch(serv, bob.srv_fd, "NICK bobby");

	std::string alice_sees = drain(alice.cap_fd);
	ASSERT(has(alice_sees, "NICK"));
	ASSERT(has(alice_sees, "bobby"));

	Channel* chan = serv.getChannels().get_channel("#room");
	ASSERT(chan->hasUser("bobby"));
	ASSERT(!chan->hasUser("bob"));

	// Subsequent PRIVMSG to the new nick should be delivered
	dispatch(serv, alice.srv_fd, "PRIVMSG bobby :still you?");
	std::string bob_sees = drain(bob.cap_fd);
	ASSERT(has(bob_sees, "PRIVMSG"));
	ASSERT(has(bob_sees, "still you?"));

	close_peer(alice);
	close_peer(bob);
}

// executecmd should catch Command::MalformedException and return 1 — the
// pipeline must not propagate parser exceptions out of the dispatch entry.
TEST(executecmd_malformed_returns_nonzero_and_does_not_throw)
{
	Server serv("6667", "secret");
	Peer   p = make_peer();
	add_registered(serv, p.srv_fd, "alice");

	int rc = serv.executecmd(p.srv_fd, ":onlyprefix\r\n");
	ASSERT_EQ(rc, 1);
	ASSERT(drain(p.cap_fd).empty());
	close_peer(p);
}

// PASS retry: after a wrong password attempt, the client must still be able
// to authenticate with the correct one. (The handler must not lock out.)
TEST(session_pass_retry_after_wrong_password)
{
	Server serv("6667", "secret");
	Peer   p = make_peer();

	dispatch(serv, p.srv_fd, "PASS wrong");
	{
		std::string out = drain(p.cap_fd);
		ASSERT(has(out, "464"));
	}

	dispatch(serv, p.srv_fd, "PASS secret");
	dispatch(serv, p.srv_fd, "NICK alice");
	dispatch(serv, p.srv_fd, "USER alice 0 * :Alice");
	{
		std::string out = drain(p.cap_fd);
		ASSERT(has(out, "001"));
		Client* c = serv.getClients().get_client_fd(p.srv_fd);
		ASSERT(c != NULL);
		ASSERT(c->isRegistered());
	}
	close_peer(p);
}

// USER arriving before NICK: registration should only complete when the
// NICK eventually shows up.
TEST(session_user_before_nick_completes_on_nick)
{
	Server serv("6667", "secret");
	Peer   p = make_peer();

	dispatch(serv, p.srv_fd, "PASS secret");
	dispatch(serv, p.srv_fd, "USER alice 0 * :Alice");
	{
		Client* c = serv.getClients().get_client_fd(p.srv_fd);
		ASSERT(c != NULL);
		ASSERT(!c->isRegistered());
		ASSERT(drain(p.cap_fd).empty());
	}

	dispatch(serv, p.srv_fd, "NICK alice");
	{
		std::string out = drain(p.cap_fd);
		ASSERT(has(out, "001"));
		Client* c = serv.getClients().get_client_fd(p.srv_fd);
		ASSERT(c->isRegistered());
	}
	close_peer(p);
}
