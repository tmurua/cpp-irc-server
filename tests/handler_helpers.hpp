#ifndef HANDLER_HELPERS_HPP
# define HANDLER_HELPERS_HPP

# include "test_runner.hpp"
# include "Server.hpp"
# include "Reply.hpp"
# include "Clients/Client.hpp"
# include "Clients/Clients.hpp"
# include "Channels/Channel.hpp"
# include "Channels/Channels.hpp"
# include <string>
# include <vector>
# include <sys/socket.h>
# include <sys/types.h>
# include <poll.h>
# include <unistd.h>

namespace ht {

// One connected client for a handler test:
//   srv_fd: the fd the Server's sndmsg() sends to
//   cap_fd: the fd we read from to capture what the server sent
struct Peer {
	int srv_fd;
	int cap_fd;
	Peer() : srv_fd(-1), cap_fd(-1) {}
};

inline Peer make_peer() {
	int fds[2];
	socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
	Peer p;
	p.cap_fd = fds[0];
	p.srv_fd = fds[1];
	return p;
}

inline void close_peer(Peer& p) {
	if (p.cap_fd >= 0) { close(p.cap_fd); p.cap_fd = -1; }
	if (p.srv_fd >= 0) { close(p.srv_fd); p.srv_fd = -1; }
}

// Read everything currently in the capture buffer (short timeout, non-blocking).
inline std::string drain(int cap_fd, int timeout_ms = 30) {
	std::string out;
	char buf[4096];
	struct pollfd pfd;
	pfd.fd = cap_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	while (poll(&pfd, 1, timeout_ms) > 0 && (pfd.revents & POLLIN)) {
		ssize_t n = recv(cap_fd, buf, sizeof(buf), 0);
		if (n <= 0) break;
		out.append(buf, n);
		pfd.revents = 0;
	}
	return out;
}

inline bool has(const std::string& s, const std::string& sub) {
	return s.find(sub) != std::string::npos;
}

// Count how many CRLF-terminated lines are in a chunk.
inline std::size_t line_count(const std::string& s) {
	std::size_t n = 0;
	for (std::size_t i = 0; i + 1 < s.size(); i++)
		if (s[i] == '\r' && s[i + 1] == '\n') ++n;
	return n;
}

// Add a client to the Server's Clients container in registered state.
inline Client* add_registered(Server& serv, int fd, const std::string& nick) {
	Client c(fd);
	c.setNickname(nick);
	c.setUsername(nick);
	c.setHostname("localhost");
	c.setAuthenticated(true);
	c.setRegistered(true);
	serv.getClients().add_client(c);
	return serv.getClients().get_client_fd(fd);
}

// Get-or-create a channel.
inline Channel* ensure_channel(Server& serv, const std::string& name) {
	Channel* chan = serv.getChannels().get_channel(name);
	if (!chan) {
		Channel tmp(name);
		serv.getChannels().add_channel(tmp);
		chan = serv.getChannels().get_channel(name);
	}
	return chan;
}

// Put a registered client into a channel; optionally as op.
inline Client* add_in_channel(Server& serv, int fd, const std::string& nick,
                              const std::string& channame, bool as_op = false) {
	Client*  c    = add_registered(serv, fd, nick);
	Channel* chan = ensure_channel(serv, channame);
	chan->addUser(nick);
	if (as_op) chan->addOp(nick);
	c->joinChannel(channame);
	return c;
}

// Convenience wrapper around Server::executecmd that adds CRLF.
inline int dispatch(Server& serv, int fd, const std::string& line) {
	return serv.executecmd(fd, line + "\r\n");
}

} // namespace ht

#endif
