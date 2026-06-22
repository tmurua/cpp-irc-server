/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/04 17:43:29 by tmurua            #+#    #+#             */
/*   Updated: 2026/06/21 17:10:27 by tmurua           ###   ########.fr       */
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
# include <map>		// std::map

class Server {
private:
	int							_port;
	std::string					_password;
	int							_serverSocket;
	std::vector<pollfd>			_pollFds;
	std::map<int, std::string>	_receiveBuffers;
	std::map<int, std::string>	_sendBuffers;
	std::map<int, std::string>	_peerHostnames;
	std::string					_serverName;
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
	void	bufferClientInput(int clientSocket, const char* data, 
						  std::size_t dataLength);
	void	processClientBuffer(int clientSocket);
	void	setWriteMonitoring(int clientSocket, bool enabled);
	bool	sendBufferedData(std::size_t pollIndex);
	void	flushSendBuffer(int clientSocket);
	void	handleCompleteCommand(int clientSocket,
			const std::string& command);

public:
	Server(const std::string& port, const std::string& password);
	~Server();

	void	start();
	void	queueMessage(int clientSocket, const std::string& message);
	int		executecmd(int fd, const std::string& raw);
	int		sndmsg(const Client& clnt, const std::string& msg);
	void	disconnectClient(int fd);

	// Accessors used by command handlers (see include/Command/Handlers.hpp)
	Clients&				getClients();
	Channels&				getChannels();
	const std::string&		getPassword() const;
	const std::string&		getServerName() const;
	std::string				getPeerHostname(int fd) const;
};

# endif
