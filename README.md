*Created as part of the 42 curriculum by johartma and tmurua.*

# C++ IRC Server

A C++98 IRC server using non-blocking sockets and a single `poll()` event loop. Developed collaboratively at 42 Berlin under the original project name `ft_irc`.

## This repository

This is my copy of the original repository. The original repository is [Hekundan/ft_irc](https://github.com/Hekundan/ft_irc), which is currently private. The original authorship and development history are retained.

## My contribution

My work focused on the networking and server transport:

- Socket setup, non-blocking connections and the `poll()` event loop.
- Per-client input/output buffers, fragmented command reconstruction and partial sends.
- Client connection lifecycle and cleanup.

The IRC command handlers, client/channel model and protocol behavior were collaborative work. This project was built by `johartma` and `tmurua`; it is not presented as a solo implementation.

## Build and run

Requirements: a POSIX environment, Make and a C++ compiler supporting C++98. The build uses `-Wall -Wextra -Werror -std=c++98`.

```sh
git clone https://github.com/tmurua/cpp-irc-server.git
cd cpp-irc-server
make
./ircserv 6667 demo-password
```

The executable remains `ircserv`. It listens on all local IPv4 interfaces on the selected port.

Connect with [irssi](https://irssi.org/), the reference client used during development:

```text
/connect localhost 6667 demo-password
/join #lobby
/msg alice hello
```

Use a second client to join the same channel and exchange messages.

## How it works

The server tracks sockets in a `poll()` loop. Bytes received from each TCP stream are buffered until a complete IRC command is available. Replies are buffered so that sending can continue when a write sends only part of a message. Connection cleanup removes client state when clients disconnect.

Features include registration, direct/channel messages, channel joining and leaving, invitations, topic changes, kicking and channel modes. See the [original documentation](README.42.md#features) for the supported commands.

## Scope

This is an academic server implementation, not a claim of complete IRC RFC compliance or production readiness. It implements the server only. Server-to-server links, DCC, `OPER`, `WHOIS` and setting ban entries are not implemented.

## Provenance and resources

Original project: `ft_irc`, by `johartma` and `tmurua`. The [original documentation](README.42.md) retains protocol references, build details and AI-use disclosures.
