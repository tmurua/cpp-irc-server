/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/04 17:43:29 by tmurua            #+#    #+#             */
/*   Updated: 2026/06/09 07:30:30 by tmurua           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include "Clients/Clients.hpp"
# include "Channels/Channels.hpp"
# include <poll.h>	// pollfd, POLLIN
# include <vector>	// std::vector
# include <cstddef>	// std::size_t

class Server {
private:
	int					_port;
	std::string			_password;
	int					_serverSocket;
	std::vector<pollfd>	_pollFds;
	Channels			_channels;
	Clients				_clients;

	int		parsePort(const std::string& port) const;
	bool	isOnlyDigits(const std::string& text) const;
	void	createSocket();
	void	enableAddressReuse();
	void	bindSocketToPort();
	void	startListening();
	void	setSocketNonBlocking(int socketFd);
	void	addSocketToPoll(int socketFd);
	void	runEventLoop();
	bool	handlePollEvent(std::size_t pollIndex);
	void	handleServerEvent(short returnedEvents);
	bool	handleClientEvent(std::size_t pollIndex, short returnedEvents);
	void	acceptNewClient();
	bool	readFromClient(std::size_t pollIndex);
	void	disconnectClient(std::size_t pollIndex);

public:
	Server(const std::string& port, const std::string& password);
	~Server();

	void	start();
	int		executecmd(int fd, const std::string& raw);
	int		sndmsg(const Client& clnt, const std::string& msg);
	void	disconnectClient(int fd);

	// Accessors used by command handlers (see include/Command/Handlers.hpp)
	Clients&				getClients();
	Channels&				getChannels();
	const std::string&		getPassword() const;
};

# endif
