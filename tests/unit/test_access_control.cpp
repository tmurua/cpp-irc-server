// Membership + privilege checks across PRIVMSG/NOTICE/PART/TOPIC/KICK/INVITE/MODE.

#include "handler_helpers.hpp"

using namespace ht;

// ── PRIVMSG / NOTICE: membership required to write to a channel ──────────────

TEST(ac_privmsg_member_can_send_to_channel)
{
	Server serv("6667", "secret");
	Peer alice = make_peer();
	Peer bob   = make_peer();
	add_in_channel(serv, alice.srv_fd, "alice", "#room");
	add_in_channel(serv, bob.srv_fd,   "bob",   "#room");
	dispatch(serv, alice.srv_fd, "PRIVMSG #room :hi");
	ASSERT(drain(alice.cap_fd).empty());
	ASSERT(has(drain(bob.cap_fd), "hi"));
	close_peer(alice);
	close_peer(bob);
}

TEST(ac_privmsg_non_member_rejected_404)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer b = make_peer();
	add_registered(serv, a.srv_fd, "alice");
	add_in_channel(serv, b.srv_fd, "bob", "#room");
	dispatch(serv, a.srv_fd, "PRIVMSG #room :hi");
	ASSERT(has(drain(a.cap_fd), "404"));
	ASSERT(drain(b.cap_fd).empty());
	close_peer(a); close_peer(b);
}

TEST(ac_notice_non_member_silently_dropped)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer b = make_peer();
	add_registered(serv, a.srv_fd, "alice");
	add_in_channel(serv, b.srv_fd, "bob", "#room");
	dispatch(serv, a.srv_fd, "NOTICE #room :hi");
	ASSERT(drain(a.cap_fd).empty());
	ASSERT(drain(b.cap_fd).empty());
	close_peer(a); close_peer(b);
}

TEST(ac_privmsg_after_part_rejected_404)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer b = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room");
	add_in_channel(serv, b.srv_fd, "bob",   "#room");
	dispatch(serv, a.srv_fd, "PART #room");
	drain(a.cap_fd); drain(b.cap_fd);
	dispatch(serv, a.srv_fd, "PRIVMSG #room :still here?");
	ASSERT(has(drain(a.cap_fd), "404"));
	ASSERT(drain(b.cap_fd).empty());
	close_peer(a); close_peer(b);
}

TEST(ac_privmsg_to_amp_channel_non_member_rejected_404)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer b = make_peer();
	add_registered(serv, a.srv_fd, "alice");
	add_in_channel(serv, b.srv_fd, "bob", "&local");
	dispatch(serv, a.srv_fd, "PRIVMSG &local :hi");
	ASSERT(has(drain(a.cap_fd), "404"));
	close_peer(a); close_peer(b);
}

// ── PART: must be on the channel ─────────────────────────────────────────────

TEST(ac_part_non_member_sends_442)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer b = make_peer();
	add_registered(serv, a.srv_fd, "alice");
	add_in_channel(serv, b.srv_fd, "bob", "#room");
	dispatch(serv, a.srv_fd, "PART #room");
	ASSERT(has(drain(a.cap_fd), "442"));
	close_peer(a); close_peer(b);
}

TEST(ac_part_unknown_channel_sends_403)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "PART #ghost");
	ASSERT(has(drain(p.cap_fd), "403"));
	close_peer(p);
}

// ── TOPIC: membership; +t requires op to set ─────────────────────────────────

TEST(ac_topic_non_member_query_sends_442)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer b = make_peer();
	add_registered(serv, a.srv_fd, "alice");
	add_in_channel(serv, b.srv_fd, "bob", "#room");
	dispatch(serv, a.srv_fd, "TOPIC #room");
	ASSERT(has(drain(a.cap_fd), "442"));
	close_peer(a); close_peer(b);
}

TEST(ac_topic_non_member_set_sends_442)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer b = make_peer();
	add_registered(serv, a.srv_fd, "alice");
	add_in_channel(serv, b.srv_fd, "bob", "#room");
	dispatch(serv, a.srv_fd, "TOPIC #room :new topic");
	ASSERT(has(drain(a.cap_fd), "442"));
	Channel* c = serv.getChannels().get_channel("#room");
	ASSERT(!c->hasTopic());
	close_peer(a); close_peer(b);
}

TEST(ac_topic_member_can_set_when_not_restricted)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room");
	dispatch(serv, a.srv_fd, "TOPIC #room :greetings");
	drain(a.cap_fd);
	Channel* c = serv.getChannels().get_channel("#room");
	ASSERT(c->hasTopic());
	ASSERT(c->getTopic() == "greetings");
	close_peer(a);
}

TEST(ac_topic_non_op_blocked_when_restricted_482)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room");
	serv.getChannels().get_channel("#room")->setTopicRestricted(true);
	dispatch(serv, a.srv_fd, "TOPIC #room :nope");
	ASSERT(has(drain(a.cap_fd), "482"));
	ASSERT(!serv.getChannels().get_channel("#room")->hasTopic());
	close_peer(a);
}

TEST(ac_topic_op_can_set_when_restricted)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room", true);
	serv.getChannels().get_channel("#room")->setTopicRestricted(true);
	dispatch(serv, a.srv_fd, "TOPIC #room :allowed");
	drain(a.cap_fd);
	ASSERT(serv.getChannels().get_channel("#room")->getTopic() == "allowed");
	close_peer(a);
}

// ── KICK: requires membership + op ───────────────────────────────────────────

TEST(ac_kick_non_member_sender_sends_442)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer b = make_peer();
	add_registered(serv, a.srv_fd, "alice");
	add_in_channel(serv, b.srv_fd, "bob", "#room");
	dispatch(serv, a.srv_fd, "KICK #room bob");
	ASSERT(has(drain(a.cap_fd), "442"));
	ASSERT(serv.getChannels().get_channel("#room")->hasUser("bob"));
	close_peer(a); close_peer(b);
}

TEST(ac_kick_non_op_member_sends_482)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer b = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room");
	add_in_channel(serv, b.srv_fd, "bob",   "#room");
	dispatch(serv, a.srv_fd, "KICK #room bob");
	ASSERT(has(drain(a.cap_fd), "482"));
	ASSERT(serv.getChannels().get_channel("#room")->hasUser("bob"));
	close_peer(a); close_peer(b);
}

TEST(ac_kick_op_can_kick_member)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer b = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room", true);
	add_in_channel(serv, b.srv_fd, "bob",   "#room");
	dispatch(serv, a.srv_fd, "KICK #room bob :bye");
	drain(a.cap_fd);
	ASSERT(has(drain(b.cap_fd), "KICK"));
	ASSERT(!serv.getChannels().get_channel("#room")->hasUser("bob"));
	close_peer(a); close_peer(b);
}

TEST(ac_kick_op_target_not_on_channel_sends_441)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room", true);
	dispatch(serv, a.srv_fd, "KICK #room ghost");
	ASSERT(has(drain(a.cap_fd), "441"));
	close_peer(a);
}

TEST(ac_kick_unknown_channel_sends_403)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "KICK #ghost bob");
	ASSERT(has(drain(p.cap_fd), "403"));
	close_peer(p);
}

// ── INVITE: sender must be on channel; +i needs op ───────────────────────────

TEST(ac_invite_non_member_sender_sends_442)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer b = make_peer();
	add_registered(serv, a.srv_fd, "alice");
	add_in_channel(serv, b.srv_fd, "bob", "#room");
	add_registered(serv, -1, "carol");
	dispatch(serv, a.srv_fd, "INVITE carol #room");
	ASSERT(has(drain(a.cap_fd), "442"));
	ASSERT(!serv.getChannels().get_channel("#room")->isInvited("carol"));
	close_peer(a); close_peer(b);
}

TEST(ac_invite_non_op_blocked_when_invite_only_482)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer carol = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room");
	serv.getChannels().get_channel("#room")->setInviteOnly(true);
	add_registered(serv, carol.srv_fd, "carol");
	dispatch(serv, a.srv_fd, "INVITE carol #room");
	ASSERT(has(drain(a.cap_fd), "482"));
	ASSERT(!serv.getChannels().get_channel("#room")->isInvited("carol"));
	close_peer(a); close_peer(carol);
}

TEST(ac_invite_op_can_invite_when_invite_only)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer carol = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room", true);
	serv.getChannels().get_channel("#room")->setInviteOnly(true);
	add_registered(serv, carol.srv_fd, "carol");
	dispatch(serv, a.srv_fd, "INVITE carol #room");
	std::string a_out = drain(a.cap_fd);
	std::string c_out = drain(carol.cap_fd);
	ASSERT(has(a_out, "341"));
	ASSERT(has(c_out, "INVITE"));
	ASSERT(serv.getChannels().get_channel("#room")->isInvited("carol"));
	close_peer(a); close_peer(carol);
}

TEST(ac_invite_member_can_invite_when_not_invite_only)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer carol = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room");
	add_registered(serv, carol.srv_fd, "carol");
	dispatch(serv, a.srv_fd, "INVITE carol #room");
	ASSERT(has(drain(carol.cap_fd), "INVITE"));
	close_peer(a); close_peer(carol);
}

TEST(ac_invite_target_already_on_channel_sends_443)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer b = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room", true);
	add_in_channel(serv, b.srv_fd, "bob",   "#room");
	dispatch(serv, a.srv_fd, "INVITE bob #room");
	ASSERT(has(drain(a.cap_fd), "443"));
	close_peer(a); close_peer(b);
}

TEST(ac_invite_unknown_target_sends_401)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room");
	dispatch(serv, a.srv_fd, "INVITE ghost #room");
	ASSERT(has(drain(a.cap_fd), "401"));
	close_peer(a);
}

// ── MODE: query open, changes require op ─────────────────────────────────────

TEST(ac_mode_query_works_for_non_op_member)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room");
	dispatch(serv, a.srv_fd, "MODE #room");
	ASSERT(has(drain(a.cap_fd), "324"));
	close_peer(a);
}

TEST(ac_mode_change_by_non_op_sends_482)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room");
	dispatch(serv, a.srv_fd, "MODE #room +i");
	ASSERT(has(drain(a.cap_fd), "482"));
	ASSERT(!serv.getChannels().get_channel("#room")->isInviteOnly());
	close_peer(a);
}

TEST(ac_mode_change_by_op_applies)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room", true);
	dispatch(serv, a.srv_fd, "MODE #room +it");
	drain(a.cap_fd);
	Channel* c = serv.getChannels().get_channel("#room");
	ASSERT(c->isInviteOnly());
	ASSERT(c->isTopicRestricted());
	close_peer(a);
}

TEST(ac_mode_op_promote_via_o_flag)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer b = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room", true);
	add_in_channel(serv, b.srv_fd, "bob",   "#room");
	dispatch(serv, a.srv_fd, "MODE #room +o bob");
	drain(a.cap_fd); drain(b.cap_fd);
	ASSERT(serv.getChannels().get_channel("#room")->isOp("bob"));
	close_peer(a); close_peer(b);
}

TEST(ac_mode_op_demote_via_o_flag)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer b = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room", true);
	add_in_channel(serv, b.srv_fd, "bob",   "#room", true);
	dispatch(serv, a.srv_fd, "MODE #room -o bob");
	drain(a.cap_fd); drain(b.cap_fd);
	ASSERT(!serv.getChannels().get_channel("#room")->isOp("bob"));
	close_peer(a); close_peer(b);
}

TEST(ac_mode_unknown_flag_sends_472)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room", true);
	dispatch(serv, a.srv_fd, "MODE #room +Z");
	ASSERT(has(drain(a.cap_fd), "472"));
	close_peer(a);
}

TEST(ac_mode_unknown_channel_sends_403)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	add_registered(serv, p.srv_fd, "alice");
	dispatch(serv, p.srv_fd, "MODE #ghost +i");
	ASSERT(has(drain(p.cap_fd), "403"));
	close_peer(p);
}

// ── JOIN: key, invite-only, user-limit gate ──────────────────────────────────

TEST(ac_join_invite_only_without_invite_sends_473)
{
	Server serv("6667", "secret");
	Peer owner = make_peer();
	Peer p     = make_peer();
	add_in_channel(serv, owner.srv_fd, "alice", "#room", true);
	serv.getChannels().get_channel("#room")->setInviteOnly(true);
	add_registered(serv, p.srv_fd, "bob");
	dispatch(serv, p.srv_fd, "JOIN #room");
	ASSERT(has(drain(p.cap_fd), "473"));
	ASSERT(!serv.getChannels().get_channel("#room")->hasUser("bob"));
	close_peer(owner); close_peer(p);
}

TEST(ac_join_invite_only_with_invite_succeeds)
{
	Server serv("6667", "secret");
	Peer owner = make_peer();
	Peer p     = make_peer();
	add_in_channel(serv, owner.srv_fd, "alice", "#room", true);
	Channel* c = serv.getChannels().get_channel("#room");
	c->setInviteOnly(true);
	c->addInvite("bob");
	add_registered(serv, p.srv_fd, "bob");
	dispatch(serv, p.srv_fd, "JOIN #room");
	drain(p.cap_fd);
	ASSERT(c->hasUser("bob"));
	close_peer(owner); close_peer(p);
}

TEST(ac_join_wrong_key_sends_475)
{
	Server serv("6667", "secret");
	Peer owner = make_peer();
	Peer p     = make_peer();
	add_in_channel(serv, owner.srv_fd, "alice", "#room", true);
	serv.getChannels().get_channel("#room")->setKey("sesame");
	add_registered(serv, p.srv_fd, "bob");
	dispatch(serv, p.srv_fd, "JOIN #room wrong");
	ASSERT(has(drain(p.cap_fd), "475"));
	close_peer(owner); close_peer(p);
}

TEST(ac_join_full_channel_sends_471)
{
	Server serv("6667", "secret");
	Peer owner = make_peer();
	Peer p     = make_peer();
	add_in_channel(serv, owner.srv_fd, "alice", "#room", true);
	serv.getChannels().get_channel("#room")->setUserLimit(1);
	add_registered(serv, p.srv_fd, "bob");
	dispatch(serv, p.srv_fd, "JOIN #room");
	ASSERT(has(drain(p.cap_fd), "471"));
	close_peer(owner); close_peer(p);
}

// ── Unregistered clients can't perform any of these ──────────────────────────

TEST(ac_unregistered_privmsg_silent)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "PRIVMSG #room :hi");
	ASSERT(drain(p.cap_fd).empty());
	close_peer(p);
}

TEST(ac_unregistered_topic_silent)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "TOPIC #room :hi");
	ASSERT(drain(p.cap_fd).empty());
	close_peer(p);
}

TEST(ac_unregistered_kick_silent)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "KICK #room bob");
	ASSERT(drain(p.cap_fd).empty());
	close_peer(p);
}

TEST(ac_unregistered_mode_silent)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "MODE #room +i");
	ASSERT(drain(p.cap_fd).empty());
	close_peer(p);
}

// ── Regression: KICK over a multi-target list must not UAF the channel ───────

TEST(bug_kick_emptying_channel_with_more_targets_does_not_crash)
{
	Server serv("6667", "secret");
	Peer op  = make_peer();
	Peer bob = make_peer();
	add_in_channel(serv, op.srv_fd,  "alice", "#room", true);
	add_in_channel(serv, bob.srv_fd, "bob",   "#room");
	// After kicking bob the channel only has alice. Then we kick alice (the
	// op herself), emptying the channel. A trailing target "ghost" must not
	// dereference the freed Channel*.
	dispatch(serv, op.srv_fd, "KICK #room bob,alice,ghost :bye");
	// If we got here without crashing, the use-after-free is gone.
	ASSERT(serv.getChannels().get_channel("#room") == NULL);
	close_peer(op); close_peer(bob);
}

// ── Regression: MODE +k / +l with no arg must NOT report the mode applied ────

TEST(bug_mode_plus_k_without_arg_does_not_set_or_report)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer b = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room", true);
	add_in_channel(serv, b.srv_fd, "bob",   "#room");
	dispatch(serv, a.srv_fd, "MODE #room +k");
	std::string a_out = drain(a.cap_fd);
	std::string b_out = drain(b.cap_fd);
	ASSERT(!serv.getChannels().get_channel("#room")->hasKey());
	// No MODE relay should advertise a +k change.
	ASSERT(!has(a_out, "MODE #room +k"));
	ASSERT(!has(b_out, "MODE #room +k"));
	close_peer(a); close_peer(b);
}

TEST(bug_mode_plus_l_without_arg_does_not_set_or_report)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room", true);
	dispatch(serv, a.srv_fd, "MODE #room +l");
	std::string a_out = drain(a.cap_fd);
	ASSERT(serv.getChannels().get_channel("#room")->getUserLimit() == 0);
	ASSERT(!has(a_out, "MODE #room +l"));
	close_peer(a);
}

TEST(bug_mode_plus_k_with_arg_still_works)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room", true);
	dispatch(serv, a.srv_fd, "MODE #room +k sesame");
	drain(a.cap_fd);
	Channel* c = serv.getChannels().get_channel("#room");
	ASSERT(c->hasKey());
	ASSERT(c->getKey() == "sesame");
	close_peer(a);
}

TEST(bug_mode_minus_k_clears_and_reports)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room", true);
	serv.getChannels().get_channel("#room")->setKey("sesame");
	dispatch(serv, a.srv_fd, "MODE #room -k");
	std::string out = drain(a.cap_fd);
	ASSERT(!serv.getChannels().get_channel("#room")->hasKey());
	ASSERT(has(out, "MODE #room -k"));
	close_peer(a);
}

// ── Regression: USER must not let the client overwrite its hostname ──────────

// ── Regression: JOIN on a channel you're already in must be a no-op ──────────

TEST(bug_join_when_already_member_does_not_rebroadcast)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	Peer b = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room");
	add_in_channel(serv, b.srv_fd, "bob",   "#room");
	dispatch(serv, a.srv_fd, "JOIN #room");
	std::string a_out = drain(a.cap_fd);
	std::string b_out = drain(b.cap_fd);
	ASSERT(a_out.empty());
	ASSERT(b_out.empty());
	close_peer(a); close_peer(b);
}

// ── Regression: MODE +o for a non-member sends 441 and does not promote ──────

TEST(bug_mode_plus_o_non_member_sends_441)
{
	Server serv("6667", "secret");
	Peer op = make_peer();
	add_in_channel(serv, op.srv_fd, "alice", "#room", true);
	add_registered(serv, -1, "ghost");
	dispatch(serv, op.srv_fd, "MODE #room +o ghost");
	std::string out = drain(op.cap_fd);
	ASSERT(has(out, "441"));
	ASSERT(!serv.getChannels().get_channel("#room")->isOp("ghost"));
	// No phantom MODE relay either.
	ASSERT(!has(out, "MODE #room +o"));
	close_peer(op);
}

// ── Regression: NICK change migrates invite-list entries ─────────────────────

TEST(bug_nick_change_preserves_invite)
{
	Server serv("6667", "secret");
	Peer op = make_peer();
	Peer p  = make_peer();
	add_in_channel(serv, op.srv_fd, "alice", "#room", true);
	Channel* c = serv.getChannels().get_channel("#room");
	c->setInviteOnly(true);
	// Register the guest as "bob" and invite them.
	dispatch(serv, p.srv_fd, "PASS secret");
	dispatch(serv, p.srv_fd, "NICK bob");
	dispatch(serv, p.srv_fd, "USER bob 0 * :Bob");
	c->addInvite("bob");
	drain(p.cap_fd);
	// bob renames himself before joining.
	dispatch(serv, p.srv_fd, "NICK robert");
	drain(p.cap_fd);
	ASSERT(!c->isInvited("bob"));
	ASSERT(c->isInvited("robert"));
	dispatch(serv, p.srv_fd, "JOIN #room");
	ASSERT(c->hasUser("robert"));
	close_peer(op); close_peer(p);
}

// ── Regression: TOPIC with explicit empty trailing clears the topic ──────────

TEST(bug_topic_explicit_empty_trailing_clears)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room");
	serv.getChannels().get_channel("#room")->setTopic("hello");
	dispatch(serv, a.srv_fd, "TOPIC #room :");
	drain(a.cap_fd);
	ASSERT(!serv.getChannels().get_channel("#room")->hasTopic());
	close_peer(a);
}

TEST(bug_topic_no_trailing_is_query_not_clear)
{
	Server serv("6667", "secret");
	Peer a = make_peer();
	add_in_channel(serv, a.srv_fd, "alice", "#room");
	serv.getChannels().get_channel("#room")->setTopic("hello");
	dispatch(serv, a.srv_fd, "TOPIC #room");
	std::string out = drain(a.cap_fd);
	ASSERT(has(out, "332"));
	ASSERT(serv.getChannels().get_channel("#room")->hasTopic());
	close_peer(a);
}

TEST(bug_user_does_not_trust_client_hostname)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	// Drive the client through PASS/NICK/USER so we exercise the real path.
	dispatch(serv, p.srv_fd, "PASS secret");
	dispatch(serv, p.srv_fd, "NICK alice");
	dispatch(serv, p.srv_fd, "USER alice spoofed.evil.host spoofed.server :Real Name");
	drain(p.cap_fd);
	Client* c = serv.getClients().get_client("alice");
	ASSERT(c != NULL);
	ASSERT(c->getHostname() != "spoofed.evil.host");
	ASSERT(!c->getHostname().empty());
	close_peer(p);
}

// ── ERR_NOTREGISTERED (451): partially-registered clients get 451, not silence ─

TEST(ac_partial_registration_join_sends_451)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "NICK alice");
	(void)drain(p.cap_fd);
	dispatch(serv, p.srv_fd, "JOIN #room");
	ASSERT(has(drain(p.cap_fd), "451"));
	close_peer(p);
}

TEST(ac_partial_registration_privmsg_sends_451)
{
	Server serv("6667", "secret");
	Peer p = make_peer();
	dispatch(serv, p.srv_fd, "NICK alice");
	(void)drain(p.cap_fd);
	dispatch(serv, p.srv_fd, "PRIVMSG bob :hi");
	ASSERT(has(drain(p.cap_fd), "451"));
	close_peer(p);
}
