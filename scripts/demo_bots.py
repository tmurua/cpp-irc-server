#!/usr/bin/env python3
"""Start ircserv and connect demo bots that auto-reply to PRIVMSG."""

from __future__ import print_function

import argparse
import os
import signal
import socket
import subprocess
import sys
import threading
import time


class IrcBot(object):
    def __init__(self, host, port, password, nick, channels, reply_template, peer_nicks):
        self.host = host
        self.port = port
        self.password = password
        self.nick = nick
        self.channels = channels
        self.reply_template = reply_template
        self.peer_nicks = peer_nicks
        self.sock = None
        self.buffer = ""
        self.running = False
        self.thread = None

    def connect(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.host, self.port))
        self._send("PASS %s" % self.password)
        self._send("NICK %s" % self.nick)
        self._send("USER %s 0 * :%s bot" % (self.nick, self.nick))
        for channel in self.channels:
            self._send("JOIN %s" % channel)
        self._wait_for_registration()

    def _send(self, line):
        self.sock.sendall((line + "\r\n").encode("utf-8"))

    def _wait_for_registration(self):
        deadline = time.time() + 5.0
        while time.time() < deadline:
            for line in self._read_lines():
                if " 001 " in line or " 004 " in line:
                    return
        raise RuntimeError("%s: registration timed out" % self.nick)

    def _read_lines(self):
        try:
            data = self.sock.recv(4096)
        except socket.error:
            return []
        if not data:
            return []
        self.buffer += data.decode("utf-8", errors="replace")
        lines = []
        while "\r\n" in self.buffer:
            line, self.buffer = self.buffer.split("\r\n", 1)
            if line:
                lines.append(line)
        return lines

    def start(self):
        self.running = True
        self.thread = threading.Thread(target=self._loop, name=self.nick)
        self.thread.daemon = True
        self.thread.start()

    def stop(self):
        self.running = False
        if self.sock is not None:
            try:
                self._send("QUIT :demo shutdown")
            except socket.error:
                pass
            try:
                self.sock.close()
            except socket.error:
                pass
            self.sock = None

    def _loop(self):
        while self.running:
            lines = self._read_lines()
            if not lines and not self.running:
                break
            for line in lines:
                self._handle_line(line)
            if not lines:
                time.sleep(0.05)

    def _handle_line(self, line):
        if line.startswith("PING "):
            self._send("PONG " + line[5:])
            return
        if " PRIVMSG " not in line:
            return

        prefix, rest = line.split(" PRIVMSG ", 1)
        if not prefix.startswith(":") or "!" not in prefix:
            return
        sender = prefix[1:].split("!", 1)[0]
        if sender == self.nick or sender in self.peer_nicks:
            return

        target, _, text = rest.partition(" :")
        if not text:
            return

        if target != self.nick and target not in self.channels:
            return

        reply = self.reply_template % {
            "nick": self.nick,
            "sender": sender,
            "target": target,
            "text": text,
        }
        reply_target = target if target.startswith("#") else sender
        self._send("PRIVMSG %s :%s" % (reply_target, reply))


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
    deadline = time.time() + 5.0
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
    raise RuntimeError("ircserv did not become ready on port %s" % port)


def main():
    parser = argparse.ArgumentParser(
        description="Start ircserv with auto-replying demo bots."
    )
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=6668)
    parser.add_argument("--password", default="secretpass")
    parser.add_argument(
        "--no-server",
        action="store_true",
        help="Do not start ircserv; connect to an existing server.",
    )
    args = parser.parse_args()

    root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    server_proc = None
    bots = []

    bot_specs = [
        ("alice", ["#lobby", "#help"], "Hi %(sender)s, %(nick)s here. You said: %(text)s"),
        ("bob", ["#lobby", "#random"], "Hey %(sender)s, %(nick)s got your message: %(text)s"),
        ("carol", ["#help", "#random"], "%(nick)s speaking - thanks %(sender)s: %(text)s"),
    ]
    bot_nicks = set(nick for nick, _, _ in bot_specs)

    def shutdown(signum=None, frame=None):
        print("\nShutting down...")
        for bot in bots:
            bot.stop()
        if server_proc is not None and server_proc.poll() is None:
            server_proc.terminate()
            try:
                server_proc.wait(timeout=3)
            except subprocess.TimeoutExpired:
                server_proc.kill()
        sys.exit(0)

    signal.signal(signal.SIGINT, shutdown)
    signal.signal(signal.SIGTERM, shutdown)

    if not args.no_server:
        ircserv = find_ircserv(root)
        if ircserv is None:
            print("ircserv not found, building...")
            build_ircserv(root)
            ircserv = find_ircserv(root)
        if ircserv is None:
            print("Could not build ircserv.", file=sys.stderr)
            return 1
        print("Starting ircserv on port %s..." % args.port, flush=True)
        server_proc = start_server(ircserv, args.port, args.password)

    print("Connecting bots...", flush=True)
    for nick, channels, template in bot_specs:
        bot = IrcBot(
            args.host,
            args.port,
            args.password,
            nick,
            channels,
            template,
            bot_nicks,
        )
        bot.connect()
        bot.start()
        bots.append(bot)
        print("  %s joined %s" % (nick, ", ".join(channels)), flush=True)

    print("", flush=True)
    print("Demo is running.", flush=True)
    print("  Server:   %s:%s" % (args.host, args.port), flush=True)
    print("  Password: %s" % args.password, flush=True)
    print("  Channels: #lobby, #help, #random", flush=True)
    print("  Bots:     alice, bob, carol", flush=True)
    print("", flush=True)
    print("Connect with:", flush=True)
    print("  irssi -c %s -p %s -w %s" % (args.host, args.port, args.password), flush=True)
    print("", flush=True)
    print("Try:  /join #lobby", flush=True)
    print("      /msg alice hello", flush=True)
    print("      /msg #lobby anyone there?", flush=True)
    print("", flush=True)
    print("Press Ctrl+C to stop.", flush=True)

    while True:
        time.sleep(1)


if __name__ == "__main__":
    sys.exit(main() or 0)
