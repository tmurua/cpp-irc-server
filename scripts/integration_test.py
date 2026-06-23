#!/usr/bin/env python3
"""Integration test suite for ft_irc.

Starts ircserv (unless --no-server), connects many clients, and exercises
registration, CAP/PING, all channel modes (+i +t +k +l +o, ban list query),
JOIN/PART/KICK/INVITE/TOPIC/PRIVMSG/NOTICE/NICK/WHO, fragmented commands,
and a 20-user channel.
"""

from __future__ import print_function

import argparse
import os
import socket
import subprocess
import sys
import time


# ── Minimal IRC client ────────────────────────────────────────────────────────

class IrcClient(object):
    def __init__(self, host, port, password):
        self.host = host
        self.port = port
        self.password = password
        self.sock = None
        self.buffer = ""
        self.nick = ""

    def connect(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.host, self.port))
        self.sock.settimeout(0.2)

    def close(self):
        if self.sock is not None:
            try:
                self.send("QUIT :integration test done")
            except socket.error:
                pass
            try:
                self.sock.close()
            except socket.error:
                pass
            self.sock = None

    def send(self, line):
        self.sock.sendall((line + "\r\n").encode("utf-8"))

    def register(self, nick, realname=None, cap=True):
        self.nick = nick
        if cap:
            self.send("CAP LS 302")
            self.send("CAP END")
        self.send("PASS %s" % self.password)
        self.send("NICK %s" % nick)
        self.send("USER %s 0 * :%s" % (nick, realname or nick))
        lines = self.collect(0.5)
        joined = "\n".join(lines)
        if " 001 " not in joined:
            raise AssertionError("%s: expected 001 welcome, got: %s" % (nick, joined[:300]))
        return lines

    def collect(self, timeout=0.4):
        deadline = time.time() + timeout
        lines = []
        while time.time() < deadline:
            try:
                data = self.sock.recv(8192)
            except socket.error:
                break
            if not data:
                break
            self.buffer += data.decode("utf-8", errors="replace")
            while "\r\n" in self.buffer:
                line, self.buffer = self.buffer.split("\r\n", 1)
                if line:
                    lines.append(line)
        return lines

    def command(self, line, timeout=0.4):
        self.send(line)
        return self.collect(timeout)

    def expect(self, lines, *needles):
        joined = "\n".join(lines)
        for needle in needles:
            if needle not in joined:
                raise AssertionError("expected %r in:\n%s" % (needle, joined[:500]))

    def expect_none(self, lines, *needles):
        joined = "\n".join(lines)
        for needle in needles:
            if needle in joined:
                raise AssertionError("did not expect %r in:\n%s" % (needle, joined[:500]))


# ── Test runner ───────────────────────────────────────────────────────────────

class Suite(object):
    def __init__(self):
        self.passed = 0
        self.failed = 0
        self.errors = []

    def run(self, name, fn, host, port, password):
        clients = []
        try:
            fn(host, port, password, clients)
            self.passed += 1
            print("  PASS  %s" % name)
        except Exception as exc:
            self.failed += 1
            self.errors.append((name, exc))
            print("  FAIL  %s: %s" % (name, exc))
        finally:
            for c in clients:
                try:
                    c.close()
                except Exception:
                    pass

    def summary(self):
        total = self.passed + self.failed
        print("")
        print("=" * 60)
        print("%d passed, %d failed (of %d)" % (self.passed, self.failed, total))
        if self.errors:
            print("")
            for name, exc in self.errors:
                print("  %s: %s" % (name, exc))
        return 0 if self.failed == 0 else 1


def mk(host, port, password, clients, nick):
    c = IrcClient(host, port, password)
    c.connect()
    clients.append(c)
    return c


# ── Tests ─────────────────────────────────────────────────────────────────────

def test_registration_and_cap(host, port, password, clients):
    c = mk(host, port, password, clients, "regtest")
    lines = c.register("regtest", cap=True)
    c.expect(lines, "CAP * LS", "ft_irc.local 001", "ft_irc.local 004")
    lines = c.command("PING ft_irc.local")
    c.expect(lines, "PONG ft_irc.local")


def test_wrong_password(host, port, password, clients):
    c = mk(host, port, password, clients, "badpass")
    c.send("PASS wrong")
    c.send("NICK badpass")
    c.send("USER badpass 0 * :Bad")
    lines = c.collect(0.4)
    c.expect(lines, " 464 ")


def test_nick_in_use(host, port, password, clients):
    a = mk(host, port, password, clients, "nicka")
    b = mk(host, port, password, clients, "nickb")
    a.register("dupe")
    b.send("PASS %s" % password)
    b.send("NICK dupe")
    b.send("USER dupe 0 * :Dupe")
    lines = b.collect(0.4)
    b.expect(lines, " 433 ")


def test_join_part_privmsg(host, port, password, clients):
    alice = mk(host, port, password, clients, "alice")
    bob = mk(host, port, password, clients, "bob")
    alice.register("alice")
    bob.register("bob")
    alice.command("JOIN #pub")
    bob.command("JOIN #pub")
    out = alice.command("PRIVMSG #pub :hello room")
    alice.expect_none(out, "hello room")
    out = bob.collect(0.3)
    bob.expect(out, "PRIVMSG", "hello room")
    alice.command("PART #pub")
    out = alice.command("PRIVMSG #pub :nope")
    alice.expect(out, " 404 ")


def test_direct_privmsg(host, port, password, clients):
    a = mk(host, port, password, clients, "dm_alice")
    b = mk(host, port, password, clients, "dm_bob")
    a.register("dm_alice")
    b.register("dm_bob")
    a.command("PRIVMSG dm_bob :direct hello")
    out = b.collect(0.4)
    b.expect(out, "PRIVMSG dm_bob", "direct hello")


def test_topic_set_and_get(host, port, password, clients):
    op = mk(host, port, password, clients, "top_op")
    op.register("top_op")
    op.command("JOIN #topic")
    out = op.command("TOPIC #topic :daily standup")
    op.expect(out, "TOPIC")
    out = op.command("TOPIC #topic")
    op.expect(out, " 332 ", "daily standup")


def test_mode_topic_restricted(host, port, password, clients):
    op = mk(host, port, password, clients, "trop")
    mem = mk(host, port, password, clients, "trmem")
    op.register("trop")
    mem.register("trmem")
    op.command("JOIN #t_restrict")
    mem.command("JOIN #t_restrict")
    op.command("MODE #t_restrict +t")
    out = mem.command("TOPIC #t_restrict :blocked")
    mem.expect(out, " 482 ")
    out = op.command("TOPIC #t_restrict :allowed")
    op.expect(out, "TOPIC")


def test_mode_invite_only(host, port, password, clients):
    op = mk(host, port, password, clients, "inv_op")
    guest = mk(host, port, password, clients, "inv_guest")
    op.register("inv_op")
    guest.register("inv_guest")
    op.command("JOIN #invite")
    op.command("MODE #invite +i")
    out = guest.command("JOIN #invite")
    guest.expect(out, " 473 ")
    op.command("INVITE inv_guest #invite")
    out = guest.command("JOIN #invite")
    guest.expect(out, "JOIN #invite")
    guest.expect_none(out, " 473 ")


def test_mode_channel_key(host, port, password, clients):
    op = mk(host, port, password, clients, "key_op")
    guest = mk(host, port, password, clients, "key_guest")
    op.register("key_op")
    guest.register("key_guest")
    op.command("JOIN #keyed")
    op.command("MODE #keyed +k secretkey")
    out = guest.command("JOIN #keyed")
    guest.expect(out, " 475 ")
    out = guest.command("JOIN #keyed secretkey")
    guest.expect(out, "JOIN #keyed")


def test_mode_user_limit(host, port, password, clients):
    op = mk(host, port, password, clients, "lim_op")
    op.register("lim_op")
    op.command("JOIN #limited")
    op.command("MODE #limited +l 2")
    filler = mk(host, port, password, clients, "lim_fill")
    filler.register("lim_fill")
    out = filler.command("JOIN #limited")
    filler.expect(out, "JOIN #limited")
    overflow = mk(host, port, password, clients, "lim_ov")
    overflow.register("lim_ov")
    out = overflow.command("JOIN #limited")
    overflow.expect(out, " 471 ")


def test_mode_op_kick(host, port, password, clients):
    op = mk(host, port, password, clients, "kop")
    victim = mk(host, port, password, clients, "kvictim")
    op.register("kop")
    victim.register("kvictim")
    op.command("JOIN #kickroom")
    victim.command("JOIN #kickroom")
    out = victim.command("KICK #kickroom kvictim :bye")
    victim.expect(out, " 482 ")
    out = op.command("KICK #kickroom kvictim :bye")
    op.expect(out, "KICK")
    out = victim.collect(0.3)
    victim.expect(out, "KICK")


def test_nick_change_relay(host, port, password, clients):
    a = mk(host, port, password, clients, "nick_old")
    b = mk(host, port, password, clients, "nick_see")
    a.register("nick_old")
    b.register("nick_see")
    a.command("JOIN #nickrelay")
    b.command("JOIN #nickrelay")
    b.collect(0.2)
    out = a.command("NICK nick_new")
    out = b.collect(0.4)
    b.expect(out, "NICK", "nick_new")


def test_who_channel(host, port, password, clients):
    a = mk(host, port, password, clients, "who1")
    b = mk(host, port, password, clients, "who2")
    a.register("who1")
    b.register("who2")
    a.command("JOIN #whoroom")
    b.command("JOIN #whoroom")
    out = a.command("WHO #whoroom")
    a.expect(out, " 352 ", "who2", " 315 ")


def test_mode_ban_list_query(host, port, password, clients):
    c = mk(host, port, password, clients, "banq")
    c.register("banq")
    c.command("JOIN #banroom")
    out = c.command("MODE #banroom b")
    c.expect(out, " 368 ")
    c.expect_none(out, " 482 ")


def test_notice_silent_to_non_member(host, port, password, clients):
    insider = mk(host, port, password, clients, "not_in")
    outsider = mk(host, port, password, clients, "not_out")
    insider.register("not_in")
    outsider.register("not_out")
    insider.command("JOIN #notice")
    out = outsider.command("NOTICE #notice :silent")
    outsider.expect_none(out, " 404 ", "NOTICE")


def test_mass_twenty_users(host, port, password, clients):
    """Twenty clients join the same channel; verify NAMES and messaging."""
    nicks = ["u%02d" % i for i in range(1, 21)]
    first = mk(host, port, password, clients, nicks[0])
    first.register(nicks[0])
    first.command("JOIN #mass")
    for nick in nicks[1:]:
        c = mk(host, port, password, clients, nick)
        c.register(nick)
        c.command("JOIN #mass")
    out = first.command("PRIVMSG #mass :roll call")
    first.expect_none(out, "roll call")
    last = clients[-1]
    out = last.collect(0.5)
    last.expect(out, "roll call", "PRIVMSG")
    out = first.command("WHO #mass")
    # all 20 nicks should appear across 352 lines
    joined = "\n".join(out)
    missing = [n for n in nicks if n not in joined]
    if missing:
        raise AssertionError("WHO missing nicks: %s" % missing)


def test_user_mode_invisible(host, port, password, clients):
    c = mk(host, port, password, clients, "invis")
    c.register("invis")
    out = c.command("MODE invis +i")
    c.expect_none(out, " 403 ", " 502 ")


def test_ping_empty_token(host, port, password, clients):
    c = mk(host, port, password, clients, "pinger")
    c.register("pinger")
    out = c.command("PING")
    c.expect(out, "PONG")


def test_ampersand_channel(host, port, password, clients):
    c = mk(host, port, password, clients, "ampusr")
    c.register("ampusr")
    out = c.command("JOIN &local")
    c.expect(out, "JOIN &local")


def test_mode_query(host, port, password, clients):
    c = mk(host, port, password, clients, "modq")
    c.register("modq")
    c.command("JOIN #modquery")
    c.command("MODE #modquery +it")
    out = c.command("MODE #modquery")
    c.expect(out, " 324 ", "+it")


def test_promote_op_via_mode(host, port, password, clients):
    op = mk(host, port, password, clients, "prom_op")
    mem = mk(host, port, password, clients, "prom_mem")
    op.register("prom_op")
    mem.register("prom_mem")
    op.command("JOIN #promote")
    mem.command("JOIN #promote")
    op.command("MODE #promote +o prom_mem")
    out = mem.command("TOPIC #promote :mem sets topic")
    mem.expect(out, "TOPIC")


def test_mode_remove_key(host, port, password, clients):
    op = mk(host, port, password, clients, "unk_op")
    guest = mk(host, port, password, clients, "unk_gst")
    op.register("unk_op")
    guest.register("unk_gst")
    op.command("JOIN #unkeyed")
    op.command("MODE #unkeyed +k oldkey")
    out = guest.command("JOIN #unkeyed")
    guest.expect(out, " 475 ")
    op.command("MODE #unkeyed -k")
    out = guest.command("JOIN #unkeyed")
    guest.expect(out, "JOIN #unkeyed")


def test_mode_remove_invite(host, port, password, clients):
    op = mk(host, port, password, clients, "uninv_op")
    guest = mk(host, port, password, clients, "uninv_gs")
    op.register("uninv_op")
    guest.register("uninv_gs")
    op.command("JOIN #uninvite")
    op.command("MODE #uninvite +i")
    out = guest.command("JOIN #uninvite")
    guest.expect(out, " 473 ")
    op.command("MODE #uninvite -i")
    out = guest.command("JOIN #uninvite")
    guest.expect(out, "JOIN #uninvite")


def test_privmsg_no_such_nick(host, port, password, clients):
    c = mk(host, port, password, clients, "nosuch")
    c.register("nosuch")
    out = c.command("PRIVMSG ghost :hello")
    c.expect(out, " 401 ")


def test_privmsg_no_text(host, port, password, clients):
    c = mk(host, port, password, clients, "notext")
    c.register("notext")
    out = c.command("PRIVMSG notext")
    c.expect(out, " 412 ")


def test_combined_modes(host, port, password, clients):
    c = mk(host, port, password, clients, "combo")
    c.register("combo")
    c.command("JOIN #combo")
    c.command("MODE #combo +itk secret")
    out = c.command("MODE #combo")
    c.expect(out, " 324 ", "+itk")


def _raw_connect(host, port):
    sock = socket.create_connection((host, port))
    sock.settimeout(0.5)
    return sock


def _raw_collect(sock, timeout=0.6):
    deadline = time.time() + timeout
    chunks = []
    while time.time() < deadline:
        try:
            data = sock.recv(8192)
        except socket.error:
            break
        if not data:
            break
        chunks.append(data.decode("utf-8", errors="replace"))
    return "".join(chunks)


def test_partial_pass_registration(host, port, password, clients):
    """PASS split across two TCP writes must still register."""
    sock = _raw_connect(host, port)
    clients.append(_RawCloser(sock))
    sock.sendall(b"PAS")
    time.sleep(0.15)
    sock.sendall(("S %s\r\n" % password).encode("utf-8"))
    sock.sendall(b"NICK prt01\r\nUSER prt01 0 * :partial\r\n")
    out = _raw_collect(sock)
    if " 001 " not in out:
        raise AssertionError("expected 001 after split PASS, got: %s" % out[:300])


def test_partial_command_other_client_ok(host, port, password, clients):
    """An incomplete line on one fd must not block other clients."""
    slow = _raw_connect(host, port)
    clients.append(_RawCloser(slow))
    slow.sendall(b"JOIN #parti")  # no CRLF yet
    time.sleep(0.1)

    fast = mk(host, port, password, clients, "fast01")
    fast.register("fast01")
    fast.command("JOIN #partial")
    out = fast.command("PRIVMSG #partial :still works")
    fast.expect_none(out, "still works")
    out = fast.collect(0.3)
    # no erroneous reply to the partial line
    fast.expect_none(out, " 403 ", " 451 ", " 421 ")


def test_partial_privmsg_delivered_after_chunks(host, port, password, clients):
    """A PRIVMSG split across several writes is handled as one command."""
    sender = mk(host, port, password, clients, "chunk_s")
    receiver = mk(host, port, password, clients, "chunk_r")
    sender.register("chunk_s")
    receiver.register("chunk_r")
    sender.command("JOIN #chunks")
    receiver.command("JOIN #chunks")
    receiver.collect(0.2)

    sock = sender.sock
    parts = [
        b"PRIVMSG #chunks :hel",
        b"lo ",
        b"world\r\n",
    ]
    for part in parts:
        sock.sendall(part)
        time.sleep(0.05)
    out = receiver.collect(0.5)
    receiver.expect(out, "PRIVMSG", "hello world")


def test_disconnect_mid_command(host, port, password, clients):
    """Client killed mid-command must not break the server for new clients."""
    dead = _raw_connect(host, port)
    dead.sendall(b"PRIVMSG #ghost :unfin")
    dead.close()

    time.sleep(0.15)
    c = mk(host, port, password, clients, "after_k")
    c.register("after_k")
    out = c.command("PING ft_irc.local")
    c.expect(out, "PONG")


def test_pass_retry_after_wrong_password(host, port, password, clients):
    c = mk(host, port, password, clients, "retry")
    c.send("PASS wrong")
    out = c.collect(0.3)
    c.expect(out, " 464 ")
    c.send("PASS %s" % password)
    c.send("NICK retry")
    c.send("USER retry 0 * :Retry")
    out = c.collect(0.5)
    c.expect(out, " 001 ")


def test_user_before_nick_completes(host, port, password, clients):
    c = mk(host, port, password, clients, "ubn")
    c.send("PASS %s" % password)
    c.send("USER ubn 0 * :User Before Nick")
    out = c.collect(0.2)
    c.expect_none(out, " 001 ")
    c.send("NICK ubn")
    out = c.collect(0.5)
    c.expect(out, " 001 ")


def test_auth_needmoreparams(host, port, password, clients):
    c = mk(host, port, password, clients, "nmp")
    out = c.command("PASS")
    c.expect(out, " 461 ", "PASS")
    c.send("PASS %s" % password)
    out = c.command("NICK")
    c.expect(out, " 431 ")
    out = c.command("USER nmp")
    c.expect(out, " 461 ", "USER")


def test_nick_validation(host, port, password, clients):
    c = mk(host, port, password, clients, "nval")
    c.send("PASS %s" % password)
    out = c.command("NICK -bad")
    c.expect(out, " 432 ")
    out = c.command("NICK %s" % ("x" * 10))
    c.expect(out, " 432 ")
    out = c.command("NICK ok-nick")
    c.expect_none(out, " 432 ")
    out = c.command("USER nval 0 * :Nick Val")
    c.expect_none(out, " 461 ")


def test_already_registered_462(host, port, password, clients):
    c = mk(host, port, password, clients, "regd")
    c.register("regd")
    out = c.command("PASS %s" % password)
    c.expect(out, " 462 ")
    out = c.command("USER regd 0 * :Again")
    c.expect(out, " 462 ")


def test_partial_registration_451(host, port, password, clients):
    c = mk(host, port, password, clients, "part")
    c.send("PASS %s" % password)
    c.send("NICK part")
    out = c.command("JOIN #partial451")
    c.expect(out, " 451 ")
    out = c.command("PRIVMSG part :hi")
    c.expect(out, " 451 ")


def test_join_errors(host, port, password, clients):
    c = mk(host, port, password, clients, "jerr")
    c.register("jerr")
    out = c.command("JOIN")
    c.expect(out, " 461 ", "JOIN")
    out = c.command("JOIN not-a-channel")
    c.expect(out, " 403 ")


def test_part_errors(host, port, password, clients):
    a = mk(host, port, password, clients, "perr_a")
    b = mk(host, port, password, clients, "perr_b")
    a.register("perr_a")
    b.register("perr_b")
    out = a.command("PART #ghost")
    a.expect(out, " 403 ")
    a.command("JOIN #parterr")
    b.command("JOIN #parterr")
    a.command("PART #parterr")
    out = a.command("PART #parterr")
    a.expect(out, " 442 ")


def test_topic_query_and_clear(host, port, password, clients):
    op = mk(host, port, password, clients, "tq_op")
    mem = mk(host, port, password, clients, "tq_mem")
    op.register("tq_op")
    mem.register("tq_mem")
    op.command("JOIN #tqroom")
    out = op.command("TOPIC #tqroom")
    op.expect(out, " 331 ")
    op.command("TOPIC #tqroom :standup notes")
    out = mem.command("TOPIC #tqroom")
    mem.expect(out, " 442 ")
    mem.command("JOIN #tqroom")
    out = op.command("TOPIC #tqroom")
    op.expect(out, " 332 ", "standup notes")
    out = op.command("TOPIC #tqroom :")
    op.expect(out, "TOPIC")
    out = op.command("TOPIC #tqroom")
    op.expect(out, " 331 ")


def test_kick_target_not_in_channel(host, port, password, clients):
    op = mk(host, port, password, clients, "kop2")
    op.register("kop2")
    op.command("JOIN #kick441")
    out = op.command("KICK #kick441 ghost :bye")
    op.expect(out, " 441 ")


def test_invite_errors(host, port, password, clients):
    op = mk(host, port, password, clients, "invop")
    mem = mk(host, port, password, clients, "invmem")
    guest = mk(host, port, password, clients, "invgst")
    op.register("invop")
    mem.register("invmem")
    guest.register("invgst")
    op.command("JOIN #invroom")
    mem.command("JOIN #invroom")
    out = op.command("INVITE invmem #invroom")
    op.expect(out, " 443 ")
    out = op.command("INVITE nobody #invroom")
    op.expect(out, " 401 ")
    out = mem.command("INVITE invgst #invroom")
    mem.expect(out, " 341 ")
    out = guest.collect(0.3)
    guest.expect(out, "INVITE")


def test_mode_errors(host, port, password, clients):
    op = mk(host, port, password, clients, "mop")
    mem = mk(host, port, password, clients, "mmem")
    op.register("mop")
    mem.register("mmem")
    op.command("JOIN #moderr")
    mem.command("JOIN #moderr")
    out = mem.command("MODE")
    mem.expect(out, " 461 ", "MODE")
    out = op.command("MODE #moderr +z")
    op.expect(out, " 472 ")
    out = mem.command("MODE #moderr +i")
    mem.expect(out, " 482 ")
    out = op.command("MODE mmem +i")
    op.expect(out, " 502 ")


def test_privmsg_no_target(host, port, password, clients):
    c = mk(host, port, password, clients, "p411")
    c.register("p411")
    out = c.command("PRIVMSG")
    c.expect(out, " 411 ")


def test_unknown_command_421(host, port, password, clients):
    c = mk(host, port, password, clients, "unk")
    c.register("unk")
    out = c.command("FOOBAR")
    c.expect(out, " 421 ")


def test_cap_req_nak(host, port, password, clients):
    c = mk(host, port, password, clients, "capnak")
    c.send("PASS %s" % password)
    c.send("NICK capnak")
    out = c.command("CAP REQ :multi-prefix")
    c.expect(out, "CAP * NAK")


def test_ping_with_token(host, port, password, clients):
    c = mk(host, port, password, clients, "pingtok")
    c.register("pingtok")
    out = c.command("PING :hello-token")
    c.expect(out, "PONG", "hello-token")


def test_quit_broadcasts(host, port, password, clients):
    a = mk(host, port, password, clients, "quit_a")
    b = mk(host, port, password, clients, "quit_b")
    a.register("quit_a")
    b.register("quit_b")
    a.command("JOIN #quitroom")
    b.command("JOIN #quitroom")
    b.collect(0.2)
    a.command("QUIT :leaving now")
    out = b.collect(0.4)
    b.expect(out, "QUIT", "leaving now")


def test_join_multiple_channels(host, port, password, clients):
    c = mk(host, port, password, clients, "multi")
    c.register("multi")
    out = c.command("JOIN #one,#two")
    c.expect(out, "JOIN #one")
    c.expect(out, "JOIN #two")


def test_join_already_member_no_rebroadcast(host, port, password, clients):
    a = mk(host, port, password, clients, "reb_a")
    b = mk(host, port, password, clients, "reb_b")
    a.register("reb_a")
    b.register("reb_b")
    a.command("JOIN #rebro")
    b.command("JOIN #rebro")
    b.collect(0.2)
    a.command("JOIN #rebro")
    out = b.collect(0.3)
    b.expect_none(out, "JOIN")


def test_mode_op_demote(host, port, password, clients):
    op = mk(host, port, password, clients, "dem_op")
    mem = mk(host, port, password, clients, "dem_mem")
    op.register("dem_op")
    mem.register("dem_mem")
    op.command("JOIN #demote")
    mem.command("JOIN #demote")
    op.command("MODE #demote +o dem_mem")
    op.command("MODE #demote +t")
    op.command("MODE #demote -o dem_mem")
    out = mem.command("TOPIC #demote :blocked again")
    mem.expect(out, " 482 ")


def test_nick_change_preserves_invite(host, port, password, clients):
    op = mk(host, port, password, clients, "ninv_op")
    guest = mk(host, port, password, clients, "ninv_gs")
    op.register("ninv_op")
    guest.register("ninv_gs")
    op.command("JOIN #ninv")
    op.command("MODE #ninv +i")
    op.command("INVITE ninv_gs #ninv")
    guest.command("NICK ninv_new")
    out = guest.command("JOIN #ninv")
    guest.expect(out, "JOIN #ninv")
    guest.expect_none(out, " 473 ")


def test_mode_plus_k_without_arg(host, port, password, clients):
    c = mk(host, port, password, clients, "knoarg")
    c.register("knoarg")
    c.command("JOIN #knoarg")
    out = c.command("MODE #knoarg +k")
    c.expect_none(out, "MODE #knoarg +k")
    out = c.command("MODE #knoarg")
    c.expect_none(out, "+k")


def test_kick_multiple_targets(host, port, password, clients):
    op = mk(host, port, password, clients, "km_op")
    a = mk(host, port, password, clients, "km_a")
    b = mk(host, port, password, clients, "km_b")
    op.register("km_op")
    a.register("km_a")
    b.register("km_b")
    op.command("JOIN #kmulti")
    a.command("JOIN #kmulti")
    b.command("JOIN #kmulti")
    op.command("KICK #kmulti km_a,km_b :both out")
    out = "\n".join(a.collect(0.4)) + "\n" + "\n".join(b.collect(0.4))
    if "KICK" not in out or "km_b" not in out:
        raise AssertionError("expected KICK for both targets, got: %s" % out[:300])


def test_malformed_prefix_no_crash(host, port, password, clients):
    sock = _raw_connect(host, port)
    clients.append(_RawCloser(sock))
    sock.sendall(b":onlyprefix\r\n")
    time.sleep(0.1)
    c = mk(host, port, password, clients, "after_bad")
    c.register("after_bad")
    out = c.command("PING ft_irc.local")
    c.expect(out, "PONG")


class _RawCloser(object):
    """Wraps a raw socket so Suite.run() closes it in finally."""

    def __init__(self, sock):
        self.sock = sock

    def close(self):
        try:
            self.sock.close()
        except socket.error:
            pass


# ── Server helpers ────────────────────────────────────────────────────────────

def find_ircserv(root):
    path = os.path.join(root, "ircserv")
    if os.path.isfile(path) and os.access(path, os.X_OK):
        return path
    return None


def build_ircserv(root):
    subprocess.check_call(["make"], cwd=root)


def start_server(ircserv, port, password):
    proc = subprocess.Popen(
        [ircserv, str(port), password],
        cwd=os.path.dirname(ircserv),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    deadline = time.time() + 8.0
    while time.time() < deadline:
        try:
            sock = socket.create_connection(("127.0.0.1", port), timeout=0.2)
            sock.close()
            return proc
        except socket.error:
            if proc.poll() is not None:
                output = proc.stdout.read().decode("utf-8", errors="replace")
                raise RuntimeError("ircserv exited early:\n" + output)
            time.sleep(0.1)
    proc.terminate()
    raise RuntimeError("ircserv not ready on port %s" % port)


# ── Main ──────────────────────────────────────────────────────────────────────

TESTS = [
    ("registration and CAP/PING", test_registration_and_cap),
    ("wrong password (464)", test_wrong_password),
    ("nick in use (433)", test_nick_in_use),
    ("JOIN / PART / PRIVMSG", test_join_part_privmsg),
    ("direct PRIVMSG", test_direct_privmsg),
    ("TOPIC set and get", test_topic_set_and_get),
    ("MODE +t topic restricted", test_mode_topic_restricted),
    ("MODE +i invite only", test_mode_invite_only),
    ("MODE +k channel key", test_mode_channel_key),
    ("MODE +l user limit", test_mode_user_limit),
    ("KICK requires op", test_mode_op_kick),
    ("NICK change relay", test_nick_change_relay),
    ("WHO channel members", test_who_channel),
    ("MODE ban list query (368)", test_mode_ban_list_query),
    ("NOTICE silent to non-member", test_notice_silent_to_non_member),
    ("20 users in one channel", test_mass_twenty_users),
    ("user MODE +i", test_user_mode_invisible),
    ("empty PING", test_ping_empty_token),
    ("& local channel", test_ampersand_channel),
    ("MODE query +it", test_mode_query),
    ("MODE +o promote", test_promote_op_via_mode),
    ("MODE -k remove key", test_mode_remove_key),
    ("MODE -i remove invite", test_mode_remove_invite),
    ("PRIVMSG no such nick (401)", test_privmsg_no_such_nick),
    ("PRIVMSG no text (412)", test_privmsg_no_text),
    ("combined +itk modes", test_combined_modes),
    ("partial PASS registration", test_partial_pass_registration),
    ("partial cmd other client ok", test_partial_command_other_client_ok),
    ("partial PRIVMSG in chunks", test_partial_privmsg_delivered_after_chunks),
    ("disconnect mid-command", test_disconnect_mid_command),
    ("PASS retry after wrong password", test_pass_retry_after_wrong_password),
    ("USER before NICK completes", test_user_before_nick_completes),
    ("auth needmoreparams (461/431)", test_auth_needmoreparams),
    ("nick validation (432)", test_nick_validation),
    ("already registered (462)", test_already_registered_462),
    ("partial registration (451)", test_partial_registration_451),
    ("JOIN errors (461/403)", test_join_errors),
    ("PART errors (403/442)", test_part_errors),
    ("TOPIC query and clear", test_topic_query_and_clear),
    ("KICK target not in channel (441)", test_kick_target_not_in_channel),
    ("INVITE errors (401/443)", test_invite_errors),
    ("MODE errors (461/472/502)", test_mode_errors),
    ("PRIVMSG no target (411)", test_privmsg_no_target),
    ("unknown command (421)", test_unknown_command_421),
    ("CAP REQ NAK", test_cap_req_nak),
    ("PING with token", test_ping_with_token),
    ("QUIT broadcasts", test_quit_broadcasts),
    ("JOIN multiple channels", test_join_multiple_channels),
    ("JOIN already member no rebroadcast", test_join_already_member_no_rebroadcast),
    ("MODE -o demote", test_mode_op_demote),
    ("nick change preserves invite", test_nick_change_preserves_invite),
    ("MODE +k without arg", test_mode_plus_k_without_arg),
    ("KICK multiple targets", test_kick_multiple_targets),
    ("malformed prefix no crash", test_malformed_prefix_no_crash),
]


def main():
    parser = argparse.ArgumentParser(description="Run ft_irc integration tests.")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=16667)
    parser.add_argument("--password", default="testpass")
    parser.add_argument(
        "--no-server",
        action="store_true",
        help="Use an already-running ircserv.",
    )
    args = parser.parse_args()

    root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    server_proc = None

    if not args.no_server:
        ircserv = find_ircserv(root)
        if ircserv is None:
            print("Building ircserv...")
            build_ircserv(root)
            ircserv = find_ircserv(root)
        if ircserv is None:
            print("Could not find ircserv", file=sys.stderr)
            return 1
        print("Starting ircserv on port %s..." % args.port)
        server_proc = start_server(ircserv, args.port, args.password)

    suite = Suite()
    print("")
    print("Running %d integration tests against %s:%s" % (
        len(TESTS), args.host, args.port))
    print("")

    for name, fn in TESTS:
        suite.run(name, fn, args.host, args.port, args.password)

    if server_proc is not None and server_proc.poll() is None:
        server_proc.terminate()
        try:
            server_proc.wait(timeout=3)
        except subprocess.TimeoutExpired:
            server_proc.kill()

    return suite.summary()


if __name__ == "__main__":
    sys.exit(main())
