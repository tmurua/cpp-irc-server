*This project has been created as part of the 42 curriculum by johartma, tmurua.*

# ft_irc

`ft_irc` is a 42 School project about building an IRC server in C++98.

## Description

The goal of this project is to implement an IRC server that can communicate with a real IRC client over TCP/IP.

IRC, short for Internet Relay Chat, is a text-based protocol for real-time communication. Users can authenticate, set a nickname and username, join channels, send private messages, and communicate in public channels.

This project implements the server side only. It does not implement an IRC client and does not support server-to-server communication.

The executable is run as follows:

```bash
./ircserv <port> <password>
```

`<port>` is the port on which the server listens for incoming connections.

`<password>` is the connection password required by IRC clients.

## Instructions

### Compilation

```bash
make
```

The Makefile provides the required rules:

```bash
make
make all
make clean
make fclean
make re
```

The project is compiled with:

```bash
c++ -Wall -Wextra -Werror -std=c++98
```

### Execution

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 secretpass
```

The server listens on all local IPv4 interfaces (`0.0.0.0`) on the given port.

### Reference Client

[irssi](https://irssi.org/) is the reference client used during development.

Start the server, then connect from irssi:

```text
/connect localhost 6667 secretpass
```

Useful irssi commands once connected:

```text
/join #lobby
/msg alice hello          # opens a query window for direct messages
/rawlog open              # show raw server traffic for debugging
```

## Features

- Non-blocking I/O with a single `poll()` event loop
- Client registration: `PASS`, `NICK`, `USER`, minimal `CAP` handling
- Channels: `JOIN`, `PART`, `TOPIC`, `INVITE`, `KICK`, `MODE`
- Messaging: `PRIVMSG`, `NOTICE` (channel and direct)
- Channel modes: `+i` (invite-only), `+t` (topic restricted), `+k` (key), `+l` (user limit), `+o` (channel operator)
- First user to create a channel is automatically made operator
- `NICK` changes relayed to shared channels, `WHO`, `PING`/`PONG`
- Fragmented TCP input reassembled before command dispatch
- `&` local channels and `#` public channels

Not implemented: server-to-server links, DCC file transfer, `OPER`, `WHOIS`, and setting ban entries (`+b`).

## Technical Notes

The server must use non-blocking I/O and handle multiple clients without hanging.

All socket operations for reading, writing, listening, and accepting connections must be handled through one `poll()` call or an equivalent mechanism.

Incoming TCP data may arrive in fragments, so complete IRC commands must be rebuilt before being processed.

## Resources

### IRC Protocol

- [RFC 1459 — Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [RFC 2810 — Internet Relay Chat: Architecture](https://datatracker.ietf.org/doc/html/rfc2810)
- [RFC 2811 — Internet Relay Chat: Channel Management](https://datatracker.ietf.org/doc/html/rfc2811)
- [RFC 2812 — Internet Relay Chat: Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812)
- [RFC 2813 — Internet Relay Chat: Server Protocol](https://datatracker.ietf.org/doc/html/rfc2813)
- [dd.ircdocs.horse](https://dd.ircdocs.horse/refs/commands/user)

### Network Programming

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [socket(2)](https://man7.org/linux/man-pages/man2/socket.2.html)
- [bind(2)](https://man7.org/linux/man-pages/man2/bind.2.html)
- [listen(2)](https://man7.org/linux/man-pages/man2/listen.2.html)
- [accept(2)](https://man7.org/linux/man-pages/man2/accept.2.html)
- [recv(2)](https://man7.org/linux/man-pages/man2/recv.2.html)
- [send(2)](https://man7.org/linux/man-pages/man2/send.2.html)
- [poll(2)](https://man7.org/linux/man-pages/man2/poll.2.html)
- [fcntl(2)](https://man7.org/linux/man-pages/man2/fcntl.2.html)

### IRC Clients

- [HexChat](https://hexchat.github.io/)
- [irssi](https://irssi.org/)
- [WeeChat](https://weechat.org/)

## AI Usage

AI was used as a learning and planning assistant for:

- understanding the project subject;
- planning a possible division of work;
- explaining IRC, sockets, non-blocking I/O, and `poll()`;
- drafting documentation;
- suggesting possible test areas, implementing statemachine tests
- implementing repetetive functions (eg replies)
