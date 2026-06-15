#include "handler_helpers.hpp"

using namespace ht;

// ── JOIN ──────────────────────────────────────────────────────────────────────

TEST(handler_join_unregistered_silently_ignored)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "JOIN #room");
	std::string out = drain(p.cap_fd);
	ASSERT(out.empty());
	ASSERT(serv.getChannels().get_channel("#room") == NULL);
	close_peer(p);
}

TEST(handler_join_no_args_sends_461)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "JOIN");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "461"));
	close_peer(p);
}

TEST(handler_join_invalid_name_sends_403)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "JOIN noprefix");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "403"));
	close_peer(p);
}

TEST(handler_join_creates_channel_when_new)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "JOIN #new");
	Channel* chan = serv.getChannels().get_channel("#new");
	ASSERT(chan != NULL);
	ASSERT(chan->hasUser("alice"));
	close_peer(p);
}

TEST(handler_join_makes_first_user_op)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "JOIN #new");
	Channel* chan = serv.getChannels().get_channel("#new");
	ASSERT(chan->isOp("alice"));
	close_peer(p);
}

TEST(handler_join_sends_join_relay_and_names_to_joiner)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "JOIN #new");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "JOIN"));
	ASSERT(has(out, "#new"));
	ASSERT(has(out, "353"));
	ASSERT(has(out, "366"));
	ASSERT(has(out, "331"));
	close_peer(p);
}

TEST(handler_join_sends_topic_when_set)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	Channel* chan = ensure_channel(serv, "#room");
	chan->setTopic("Welcome here");
	dispatch(serv, p.srv_fd, "JOIN #room");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "332"));
	ASSERT(has(out, "Welcome here"));
	close_peer(p);
}

TEST(handler_join_invite_only_without_invite_sends_473)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	Channel* chan = ensure_channel(serv, "#priv");
	chan->setInviteOnly(true);
	dispatch(serv, p.srv_fd, "JOIN #priv");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "473"));
	ASSERT(!chan->hasUser("alice"));
	close_peer(p);
}

TEST(handler_join_with_invite_succeeds_and_clears_invite)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	Channel* chan = ensure_channel(serv, "#priv");
	chan->setInviteOnly(true);
	chan->addInvite("alice");
	dispatch(serv, p.srv_fd, "JOIN #priv");
	ASSERT(chan->hasUser("alice"));
	ASSERT(!chan->isInvited("alice"));
	close_peer(p);
}

TEST(handler_join_wrong_key_sends_475)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	Channel* chan = ensure_channel(serv, "#locked");
	chan->setKey("rightkey");
	dispatch(serv, p.srv_fd, "JOIN #locked badkey");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "475"));
	ASSERT(!chan->hasUser("alice"));
	close_peer(p);
}

TEST(handler_join_correct_key_succeeds)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	Channel* chan = ensure_channel(serv, "#locked");
	chan->setKey("opensesame");
	dispatch(serv, p.srv_fd, "JOIN #locked opensesame");
	ASSERT(chan->hasUser("alice"));
	close_peer(p);
}

TEST(handler_join_full_channel_sends_471)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	Channel* chan = ensure_channel(serv, "#full");
	chan->addUser("existing");
	chan->setUserLimit(1);
	dispatch(serv, p.srv_fd, "JOIN #full");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "471"));
	ASSERT(!chan->hasUser("alice"));
	close_peer(p);
}

TEST(handler_join_multiple_channels_comma_split)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "JOIN #a,#b,#c");
	ASSERT(serv.getChannels().get_channel("#a") != NULL);
	ASSERT(serv.getChannels().get_channel("#b") != NULL);
	ASSERT(serv.getChannels().get_channel("#c") != NULL);
	close_peer(p);
}

TEST(handler_join_broadcasts_to_existing_members)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room", true);
	add_registered(serv, bob.srv_fd, "bob");
	(void)drain(alice.cap_fd);
	dispatch(serv, bob.srv_fd, "JOIN #room");
	std::string alice_sees = drain(alice.cap_fd);
	ASSERT(has(alice_sees, "JOIN"));
	ASSERT(has(alice_sees, "bob"));
	close_peer(alice);
	close_peer(bob);
}

// ── PART ──────────────────────────────────────────────────────────────────────

TEST(handler_part_no_args_sends_461)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "PART");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "461"));
	close_peer(p);
}

TEST(handler_part_nonexistent_channel_sends_403)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "PART #ghost");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "403"));
	close_peer(p);
}

TEST(handler_part_when_not_member_sends_442)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	ensure_channel(serv, "#room");
	dispatch(serv, p.srv_fd, "PART #room");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "442"));
	close_peer(p);
}

TEST(handler_part_removes_user_from_channel)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room");
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room");
	dispatch(serv, alice.srv_fd, "PART #room");
	Channel* chan = serv.getChannels().get_channel("#room");
	ASSERT(chan != NULL);
	ASSERT(!chan->hasUser("alice"));
	close_peer(alice);
	close_peer(bob);
}

TEST(handler_part_broadcasts_to_members)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room");
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room");
	(void)drain(bob.cap_fd);
	dispatch(serv, alice.srv_fd, "PART #room :goodbye");
	std::string bob_sees = drain(bob.cap_fd);
	ASSERT(has(bob_sees, "PART"));
	ASSERT(has(bob_sees, "goodbye"));
	close_peer(alice);
	close_peer(bob);
}

TEST(handler_part_drops_empty_channel)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room", true);
	dispatch(serv, p.srv_fd, "PART #room");
	ASSERT(serv.getChannels().get_channel("#room") == NULL);
	close_peer(p);
}

// ── TOPIC ─────────────────────────────────────────────────────────────────────

TEST(handler_topic_no_args_sends_461)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "TOPIC");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "461"));
	close_peer(p);
}

TEST(handler_topic_nonexistent_channel_sends_403)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "TOPIC #ghost");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "403"));
	close_peer(p);
}

TEST(handler_topic_when_not_member_sends_442)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	ensure_channel(serv, "#room");
	dispatch(serv, p.srv_fd, "TOPIC #room");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "442"));
	close_peer(p);
}

TEST(handler_topic_query_no_topic_sends_331)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room");
	dispatch(serv, p.srv_fd, "TOPIC #room");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "331"));
	close_peer(p);
}

TEST(handler_topic_query_with_topic_sends_332)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room");
	serv.getChannels().get_channel("#room")->setTopic("hello world");
	dispatch(serv, p.srv_fd, "TOPIC #room");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "332"));
	ASSERT(has(out, "hello world"));
	close_peer(p);
}

TEST(handler_topic_set_restricted_non_op_sends_482)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room");
	serv.getChannels().get_channel("#room")->setTopicRestricted(true);
	dispatch(serv, p.srv_fd, "TOPIC #room :new topic");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "482"));
	close_peer(p);
}

TEST(handler_topic_set_as_op_updates_and_broadcasts)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room", true);
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room");
	serv.getChannels().get_channel("#room")->setTopicRestricted(true);
	(void)drain(bob.cap_fd);

	dispatch(serv, alice.srv_fd, "TOPIC #room :brand new");
	Channel* chan = serv.getChannels().get_channel("#room");
	ASSERT(chan->hasTopic());
	ASSERT_EQ(chan->getTopic(), std::string("brand new"));
	std::string bob_sees = drain(bob.cap_fd);
	ASSERT(has(bob_sees, "TOPIC"));
	ASSERT(has(bob_sees, "brand new"));
	close_peer(alice);
	close_peer(bob);
}

// ── KICK ──────────────────────────────────────────────────────────────────────

TEST(handler_kick_too_few_args_sends_461)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room", true);
	dispatch(serv, p.srv_fd, "KICK #room");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "461"));
	close_peer(p);
}

TEST(handler_kick_nonexistent_channel_sends_403)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "KICK #ghost bob");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "403"));
	close_peer(p);
}

TEST(handler_kick_when_not_in_channel_sends_442)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	ensure_channel(serv, "#room");
	dispatch(serv, p.srv_fd, "KICK #room bob");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "442"));
	close_peer(p);
}

TEST(handler_kick_non_op_sends_482)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room");
	dispatch(serv, p.srv_fd, "KICK #room bob");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "482"));
	close_peer(p);
}

TEST(handler_kick_target_not_in_channel_sends_441)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room", true);
	dispatch(serv, p.srv_fd, "KICK #room ghost");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "441"));
	close_peer(p);
}

TEST(handler_kick_valid_removes_target_and_broadcasts)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room", true);
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room");
	(void)drain(bob.cap_fd);

	dispatch(serv, alice.srv_fd, "KICK #room bob :rule violation");
	Channel* chan = serv.getChannels().get_channel("#room");
	ASSERT(!chan->hasUser("bob"));
	std::string bob_sees = drain(bob.cap_fd);
	ASSERT(has(bob_sees, "KICK"));
	ASSERT(has(bob_sees, "bob"));
	ASSERT(has(bob_sees, "rule violation"));
	close_peer(alice);
	close_peer(bob);
}

// ── INVITE ────────────────────────────────────────────────────────────────────

TEST(handler_invite_too_few_args_sends_461)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room", true);
	dispatch(serv, p.srv_fd, "INVITE bob");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "461"));
	close_peer(p);
}

TEST(handler_invite_unknown_target_sends_401)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room", true);
	dispatch(serv, p.srv_fd, "INVITE ghost #room");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "401"));
	close_peer(p);
}

TEST(handler_invite_nonexistent_channel_sends_403)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_registered(serv, alice.srv_fd, "alice");
	add_registered(serv, bob.srv_fd,   "bob");
	dispatch(serv, alice.srv_fd, "INVITE bob #ghost");
	std::string out = drain(alice.cap_fd);
	ASSERT(has(out, "403"));
	close_peer(alice);
	close_peer(bob);
}

TEST(handler_invite_inviter_not_in_channel_sends_442)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_registered(serv, alice.srv_fd, "alice");
	add_registered(serv, bob.srv_fd,   "bob");
	ensure_channel(serv, "#room");
	dispatch(serv, alice.srv_fd, "INVITE bob #room");
	std::string out = drain(alice.cap_fd);
	ASSERT(has(out, "442"));
	close_peer(alice);
	close_peer(bob);
}

TEST(handler_invite_target_already_member_sends_443)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room", true);
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room");
	dispatch(serv, alice.srv_fd, "INVITE bob #room");
	std::string out = drain(alice.cap_fd);
	ASSERT(has(out, "443"));
	close_peer(alice);
	close_peer(bob);
}

TEST(handler_invite_only_non_op_sends_482)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#priv");
	add_registered(serv, bob.srv_fd, "bob");
	serv.getChannels().get_channel("#priv")->setInviteOnly(true);
	dispatch(serv, alice.srv_fd, "INVITE bob #priv");
	std::string out = drain(alice.cap_fd);
	ASSERT(has(out, "482"));
	close_peer(alice);
	close_peer(bob);
}

TEST(handler_invite_valid_adds_to_list_and_replies)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room", true);
	add_registered(serv, bob.srv_fd,   "bob");
	dispatch(serv, alice.srv_fd, "INVITE bob #room");

	std::string alice_sees = drain(alice.cap_fd);
	std::string bob_sees   = drain(bob.cap_fd);
	ASSERT(has(alice_sees, "341"));
	ASSERT(has(bob_sees, "INVITE"));
	ASSERT(has(bob_sees, "#room"));
	ASSERT(serv.getChannels().get_channel("#room")->isInvited("bob"));
	close_peer(alice);
	close_peer(bob);
}

// ── MODE ──────────────────────────────────────────────────────────────────────

TEST(handler_mode_no_args_sends_461)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "MODE");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "461"));
	close_peer(p);
}

TEST(handler_mode_nonexistent_channel_sends_403)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "MODE #ghost");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "403"));
	close_peer(p);
}

TEST(handler_mode_query_sends_324)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room", true);
	serv.getChannels().get_channel("#room")->setInviteOnly(true);
	dispatch(serv, p.srv_fd, "MODE #room");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "324"));
	ASSERT(has(out, "i"));
	close_peer(p);
}

TEST(handler_mode_non_op_sends_482)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room");
	dispatch(serv, p.srv_fd, "MODE #room +i");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "482"));
	close_peer(p);
}

TEST(handler_mode_plus_i_sets_invite_only)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room", true);
	dispatch(serv, p.srv_fd, "MODE #room +i");
	ASSERT(serv.getChannels().get_channel("#room")->isInviteOnly());
	close_peer(p);
}

TEST(handler_mode_minus_i_clears_invite_only)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room", true);
	serv.getChannels().get_channel("#room")->setInviteOnly(true);
	dispatch(serv, p.srv_fd, "MODE #room -i");
	ASSERT(!serv.getChannels().get_channel("#room")->isInviteOnly());
	close_peer(p);
}

TEST(handler_mode_plus_t_sets_topic_restricted)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room", true);
	dispatch(serv, p.srv_fd, "MODE #room +t");
	ASSERT(serv.getChannels().get_channel("#room")->isTopicRestricted());
	close_peer(p);
}

TEST(handler_mode_plus_k_sets_key)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room", true);
	dispatch(serv, p.srv_fd, "MODE #room +k secret");
	Channel* chan = serv.getChannels().get_channel("#room");
	ASSERT(chan->hasKey());
	ASSERT_EQ(chan->getKey(), std::string("secret"));
	close_peer(p);
}

TEST(handler_mode_minus_k_clears_key)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room", true);
	serv.getChannels().get_channel("#room")->setKey("oldkey");
	dispatch(serv, p.srv_fd, "MODE #room -k");
	ASSERT(!serv.getChannels().get_channel("#room")->hasKey());
	close_peer(p);
}

TEST(handler_mode_plus_l_sets_limit)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room", true);
	dispatch(serv, p.srv_fd, "MODE #room +l 42");
	ASSERT_EQ(serv.getChannels().get_channel("#room")->getUserLimit(), 42);
	close_peer(p);
}

TEST(handler_mode_minus_l_clears_limit)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room", true);
	serv.getChannels().get_channel("#room")->setUserLimit(10);
	dispatch(serv, p.srv_fd, "MODE #room -l");
	ASSERT_EQ(serv.getChannels().get_channel("#room")->getUserLimit(), 0);
	close_peer(p);
}

TEST(handler_mode_plus_o_adds_op)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room", true);
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room");
	dispatch(serv, alice.srv_fd, "MODE #room +o bob");
	ASSERT(serv.getChannels().get_channel("#room")->isOp("bob"));
	close_peer(alice);
	close_peer(bob);
}

TEST(handler_mode_minus_o_removes_op)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room", true);
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room", true);
	dispatch(serv, alice.srv_fd, "MODE #room -o bob");
	ASSERT(!serv.getChannels().get_channel("#room")->isOp("bob"));
	close_peer(alice);
	close_peer(bob);
}

TEST(handler_mode_unknown_char_sends_472)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room", true);
	dispatch(serv, p.srv_fd, "MODE #room +z");
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "472"));
	close_peer(p);
}

TEST(handler_mode_change_broadcasts_to_members)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room", true);
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room");
	(void)drain(bob.cap_fd);
	dispatch(serv, alice.srv_fd, "MODE #room +i");
	std::string bob_sees = drain(bob.cap_fd);
	ASSERT(has(bob_sees, "MODE"));
	ASSERT(has(bob_sees, "+i"));
	close_peer(alice);
	close_peer(bob);
}

// ── Channel boundary cases ───────────────────────────────────────────────────

TEST(handler_join_ampersand_channel_succeeds)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "JOIN &local");
	Channel* chan = serv.getChannels().get_channel("&local");
	ASSERT(chan != NULL);
	ASSERT(chan->hasUser("alice"));
	close_peer(p);
}

TEST(handler_join_overlong_name_sends_403)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	std::string longname = "#" + std::string(201, 'x');
	dispatch(serv, p.srv_fd, "JOIN " + longname);
	std::string out = drain(p.cap_fd);
	ASSERT(has(out, "403"));
	ASSERT(serv.getChannels().get_channel(longname) == NULL);
	close_peer(p);
}

TEST(handler_join_already_member_does_not_crash)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_in_channel(serv, p.srv_fd, "alice", "#room", true);
	dispatch(serv, p.srv_fd, "JOIN #room");
	Channel* chan = serv.getChannels().get_channel("#room");
	ASSERT(chan != NULL);
	ASSERT(chan->hasUser("alice"));
	ASSERT(chan->isOp("alice"));
	close_peer(p);
}

TEST(handler_join_empty_tokens_in_comma_list_skipped)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "JOIN ,#a,,#b,");
	ASSERT(serv.getChannels().get_channel("#a") != NULL);
	ASSERT(serv.getChannels().get_channel("#b") != NULL);
	close_peer(p);
}

// Bug fix: MODE with a sign change mid-string (e.g. "+i-t") was resetting the
// 'applied' buffer on each sign character, so earlier mode letters were dropped
// from the broadcast.  Both mode letters must appear in the relay.
TEST(handler_mode_mixed_signs_broadcast_all_applied_modes)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room", true);
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room");
	Channel* chan = serv.getChannels().get_channel("#room");
	chan->setInviteOnly(true);
	chan->setTopicRestricted(true);
	(void)drain(bob.cap_fd);

	// "+i-t": set invite-only OFF is wrong — let's use "-i+t" to flip both.
	dispatch(serv, alice.srv_fd, "MODE #room -i+t");
	std::string bob_sees = drain(bob.cap_fd);

	// Both mode letters must be present in the broadcast.
	ASSERT(has(bob_sees, "i"));
	ASSERT(has(bob_sees, "t"));
	// The channel state must reflect both changes.
	ASSERT(!chan->isInviteOnly());
	ASSERT(chan->isTopicRestricted());
	close_peer(alice);
	close_peer(bob);
}

// Bug fix: redundant sign repetition ("+i+t") was also resetting applied.
TEST(handler_mode_repeated_sign_broadcasts_all_applied_modes)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room", true);
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room");
	(void)drain(bob.cap_fd);

	dispatch(serv, alice.srv_fd, "MODE #room +i+t");
	std::string bob_sees = drain(bob.cap_fd);

	ASSERT(has(bob_sees, "i"));
	ASSERT(has(bob_sees, "t"));
	Channel* chan = serv.getChannels().get_channel("#room");
	ASSERT(chan->isInviteOnly());
	ASSERT(chan->isTopicRestricted());
	close_peer(alice);
	close_peer(bob);
}

TEST(handler_kick_multiple_targets_comma_split)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	Peer carol = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room", true);
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room");
	add_in_channel(serv, carol.srv_fd, "carol", "#room");
	dispatch(serv, alice.srv_fd, "KICK #room bob,carol :cleanup");
	Channel* chan = serv.getChannels().get_channel("#room");
	ASSERT(!chan->hasUser("bob"));
	ASSERT(!chan->hasUser("carol"));
	ASSERT(chan->hasUser("alice"));
	close_peer(alice);
	close_peer(bob);
	close_peer(carol);
}

// Bug: clearing a topic must broadcast "TOPIC #channel :\r\n" (with trailing colon),
// not "TOPIC #channel\r\n" — the latter looks like a query to receiving clients.
TEST(handler_topic_clear_broadcasts_with_trailing_colon)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room", true);
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room");
	dispatch(serv, alice.srv_fd, "TOPIC #room :hello");
	(void)drain(alice.cap_fd);
	(void)drain(bob.cap_fd);

	dispatch(serv, alice.srv_fd, "TOPIC #room :");
	std::string seen_by_bob = drain(bob.cap_fd);
	ASSERT(has(seen_by_bob, "TOPIC"));
	// Must contain " :" to signal an explicit empty trailing (topic clear).
	ASSERT(has(seen_by_bob, " :"));
	close_peer(alice);
	close_peer(bob);
}
