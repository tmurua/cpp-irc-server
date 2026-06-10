/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmurua <tmurua@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/04 17:43:29 by tmurua            #+#    #+#             */
/*   Updated: 2026/06/10 19:30:30 by tmurua           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <poll.h>	// pollfd, POLLIN
#include <vector>	// std::vector
#include <cstddef>	// std::size_t
# include <map>		// std::map

class Server {
private:
	int							_port;
	std::string					_password;
	int							_serverSocket;
	std::vector<pollfd>			_pollFds;
	std::map<int, std::string>	_receiveBuffers;

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
	void	handleCompleteCommand(int clientSocket,
			const std::string& command);

public:
	Server(const std::string& port, const std::string& password);
	~Server();

	void	start();
};

#endif
