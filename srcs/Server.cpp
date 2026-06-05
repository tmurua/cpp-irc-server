/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmurua <tmurua@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/04 18:03:21 by tmurua            #+#    #+#             */
/*   Updated: 2026/06/05 22:15:10 by tmurua           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#include <cstdlib>		// std::strtol
#include <stdexcept>	// std::invalid_argument, std::runtime_error
#include <sys/socket.h>	// socket, setsocket, bind, listen 
#include <unistd.h>		// close
#include <cstring>		// std::memset
#include <netinet/in.h>	// sockaddr_in, htons, INADDR_ANY
#include <fcntl.h>		// fcntl, F_SETFL, O_NONBLOCK

Server::Server(const std::string& port, const std::string& password)
	: _port(parsePort(port)), _password(password), _serverSocket(-1) {
	if (_password.empty())
		throw std::invalid_argument("password cannot be empty");
}

Server::~Server() {
	if (_serverSocket != -1)
		close(_serverSocket);
}

void	Server::start(){
	createSocket();
	enableAddressReuse();
	bindSocketToPort();
	startListening();
	setSocketNonBlocking(_serverSocket);
}

// ask OS to create an IPv4 TCP socket(communication endpoint)
// AF_INET chooses IPv4; SOCK_STREAM requests a stream; 0 picks matching protocol
void	Server::createSocket(){
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0); // returns fd on success
	if (_serverSocket == -1) // socket() returns -1 on failure
		throw std::runtime_error("failed to create server socket");
}

// allow bind() to reuse the local port address after a recent server restart
// SOL_SOCKET chooses socket level, instead of a protocol level for instance
void	Server::enableAddressReuse(){
	int	enabled;

	enabled = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR,
			&enabled, sizeof(enabled)) == -1)
		throw std::runtime_error("failed to configure server socket");
}

// bind _serverSocket to 0.0.0.0:_port, covering all local IPv4 interfaces
void	Server::bindSocketToPort(){
	sockaddr_in	serverAddress;

	std::memset(&serverAddress, 0, sizeof(serverAddress));

	serverAddress.sin_family = AF_INET;
	// INADDR_ANY = 0.0.0.0; the wildcard for all local IPv4 addresses
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	// htons(host-to-network short): convert 16-bit port to network byte order
	serverAddress.sin_port = htons(static_cast<unsigned short>(_port));

	// bind() expects a generic sockaddr pointer, not a sockaddr_in pointer
	if (bind(_serverSocket, reinterpret_cast<sockaddr*>(&serverAddress),
			sizeof(serverAddress)) == -1)
		throw std::runtime_error("failed to bind server socket to port");
}

// turn _serverSocket into a listening socket for new client connections
void	Server::startListening(){
	// ask OS to queue the maximum number of pending connections it supports
	if (listen(_serverSocket, SOMAXCONN) == -1) 
		throw std::runtime_error("failed to listen for connections");
}

// configure the socket, so operations don't make the server wait
void	Server::setSocketNonBlocking(int socketFd){
	if (fcntl(socketFd, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("failed to set socket to non-blocking mode");
}

int	Server::parsePort(const std::string& port) const {
	long	portNumber;

	if (port.empty())
		throw std::invalid_argument("port cannot be empty");
	if (isOnlyDigits(port) == false)
		throw std::invalid_argument("port must contain only digits");

	portNumber = std::strtol(port.c_str(), NULL, 10);
	// TCP ports use 16-bit numbers: 2^16 = 65536 possible values 
	// counting starts at 0, so the range is 0 through 65535
	// port 0 asks OS to choose a port, but this server expects an explicit port
	if (portNumber < 1 || portNumber > 65535)
		throw std::invalid_argument("port must be between 1 and 65535");

	return (static_cast<int>(portNumber));
}

bool	Server::isOnlyDigits(const std::string& text) const {
	size_t	i;

	i = 0;
	while (i < text.length()){
		if (text[i] < '0' || text[i] > '9')
			return (false);
		i++;
	}
	return (true);
}
