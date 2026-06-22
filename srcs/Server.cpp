/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/04 18:03:21 by tmurua            #+#    #+#             */
/*   Updated: 2026/06/21 17:12:57 by tmurua           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Command/Command.hpp"
#include "Reply.hpp"

#include <iostream>
#include <cstddef>		// std::size_t
#include <stdexcept>	// std::invalid_argument, std::runtime_error
#include <unistd.h>		// close
#include <cstdlib>		// std::strtol
#include <sys/socket.h>	// socket, setsockopt, bind, listen, accept, send, recv
#include <cstring>		// std::memset
#include <netinet/in.h>	// sockaddr_in, htons, INADDR_ANY
#include <fcntl.h>		// fcntl, F_SETFL, O_NONBLOCK
#include <poll.h>		// pollfd, POLLIN, POLLOUT
#include <cerrno>		// errno, EAGAIN, EWOULDBLOCK
#include <arpa/inet.h>	// inet_ntop, INET_ADDRSTRLEN
#include <csignal>		// signal, SIGPIPE, SIG_IGN

Server::Server(const std::string& port, const std::string& password)
	: _port(parsePort(port)), _password(password), _serverSocket(-1),
	  _serverName("ft_irc.local") {
	if (_password.empty())
		throw std::invalid_argument("password cannot be empty");
}

Server::~Server() {
	std::size_t	i;

	i = 0;
	while (i < _pollFds.size()){
		if (_pollFds[i].fd != _serverSocket)
			close(_pollFds[i].fd);
		i++;
	}
	if (_serverSocket != -1)
		close(_serverSocket);
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
	std::size_t	i;

	i = 0;
	while (i < text.length()){
		if (text[i] < '0' || text[i] > '9')
			return (false);
		i++;
	}
	return (true);
}

void	Server::start(){
	signal(SIGPIPE, SIG_IGN);
	createSocket();
	enableAddressReuse();
	bindSocketToPort();
	startListening();
	setSocketNonBlocking(_serverSocket);
	addSocketToPoll(_serverSocket);
	runEventLoop();
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

// add the socket to the list that poll() will monitor
void	Server::addSocketToPoll(int socketFd){
	pollfd	socketEvent;

	socketEvent.fd = socketFd;
	// POLLIN asks poll() to notify us when the socket has incoming activity
	socketEvent.events = POLLIN;
	// then poll() will fill it with the events that actually ocurred
	socketEvent.revents = 0;

	_pollFds.push_back(socketEvent);
}

// wait for socket activity and pass each reported event to its handler
void	Server::runEventLoop(){
	int			readySockets;
	std::size_t	i;

	while (true){
		// -1 makes poll() wait until at least one watched socket becomes ready
		readySockets = poll(&_pollFds[0],
				static_cast<nfds_t>(_pollFds.size()), -1); // nfds_t: nb of fds
		if (readySockets == -1)
			throw std::runtime_error("poll failed");

		i = 0;
		while (i < _pollFds.size()){
			if (handlePollEvent(i) == true)
				i++;
		}
	}
}

// check if the reported event belongs to the server socket or a client socket
bool	Server::handlePollEvent(std::size_t pollIndex){
	short	returnedEvents;
	bool	clientPollStillExists;

	returnedEvents = _pollFds[pollIndex].revents;
	if (returnedEvents == 0)
		return (true);
	if (_pollFds[pollIndex].fd == _serverSocket){
		handleServerEvent(returnedEvents);
		return (true);
	}
	clientPollStillExists = handleClientEvent(pollIndex, returnedEvents);
	return (clientPollStillExists);
}

// bitwise &: non-zero when at least one requested event flag is set
void	Server::handleServerEvent(short returnedEvents){
	if (returnedEvents & (POLLERR | POLLHUP | POLLNVAL))
		throw std::runtime_error("listening socket failed");
	if (returnedEvents & POLLIN)
		acceptNewClient();
}

// handle readable data, errors, and disconnections for one client socket
bool	Server::handleClientEvent(std::size_t pollIndex, short returnedEvents){
	if (returnedEvents & POLLIN){
		// false means readFromClient() already removed this client
		if (readFromClient(pollIndex) == false)
			return (false);
	}
	if (returnedEvents & (POLLERR | POLLNVAL)){
		disconnectClient(pollIndex);
		return (false);
	}
	if (returnedEvents & POLLOUT){
		if (sendBufferedData(pollIndex) == false)
			return (false);
	}
	//	if POLLIN is also set, let recv() drain available data first;
	//	a later recv() returning 0 will disconnect the client
	if ((returnedEvents & POLLHUP) && (returnedEvents & POLLIN) == 0){
		disconnectClient(pollIndex);
		return (false);
	}
	return (true);
}

// accept all waiting client connections and register their sockets with poll()
void	Server::acceptNewClient(){
	int	clientSocket;

	// NULL: client's address info not requested
	while (true){
		clientSocket = accept(_serverSocket, NULL, NULL); // accept returns new fd
		if (clientSocket == -1){
			// EAGAIN: no client connection is ready; try accept() again later
			// EWOULDBLOCK: accept would have to wait, but socket is non-blocking
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return;
			// any other errno value represents an actual failure
			throw std::runtime_error("failed to accept client connection");
		}
		setSocketNonBlocking(clientSocket);
		addSocketToPoll(clientSocket);
		_receiveBuffers[clientSocket] = "";
		_sendBuffers[clientSocket] = "";
		{
			sockaddr_in	peerAddr;
			socklen_t	peerLen = sizeof(peerAddr);
			std::string	hostname = "127.0.0.1";

			std::memset(&peerAddr, 0, sizeof(peerAddr));
			if (getpeername(clientSocket, reinterpret_cast<sockaddr*>(&peerAddr),
					&peerLen) == 0) {
				char ip[INET_ADDRSTRLEN];
				if (inet_ntop(AF_INET, &peerAddr.sin_addr, ip, sizeof(ip)) != NULL)
					hostname = ip;
			}
			_peerHostnames[clientSocket] = hostname;
		}
		std::cout << "Client connected on fd " << clientSocket << std::endl;
	}
}

// read one available data chunk from a client socket and to client's buffer
bool	Server::readFromClient(std::size_t pollIndex){
	char	buffer[512];
	int		clientSocket;
	ssize_t	bytesRead;

	clientSocket = _pollFds[pollIndex].fd;
	bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

	if (bytesRead == 0){ // if recv() returns 0, it means is reading no more bytes
		disconnectClient(pollIndex);
		return (false);
	}
	if (bytesRead == -1){ // recv() could not read data
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return (true);
		std::cerr << "Failed to read from client fd "
			<< clientSocket << std::endl;
		disconnectClient(pollIndex);
		return (false);
	}
	bufferClientInput(clientSocket, buffer, static_cast<std::size_t>(bytesRead));
	for (std::size_t i = 0; i < _pollFds.size(); i++) {
		if (_pollFds[i].fd == clientSocket)
			return (true);
	}
	return (false);
}

// close a client socket and remove it from the poll list
void	Server::disconnectClient(std::size_t pollIndex){
	int	clientSocket = _pollFds[pollIndex].fd;

	Client* client = _clients.get_client_fd(clientSocket);
	if (client) {
		std::string nick    = client->getNickname();
		std::string quitMsg = Reply::from_client(*client, "QUIT", "", "Connection reset by peer");
		std::set<std::string> chanList = client->getChannels();
		for (std::set<std::string>::const_iterator it = chanList.begin(); it != chanList.end(); ++it) {
			Channel* chan = _channels.get_channel(*it);
			if (!chan) continue;
			const std::set<std::string> users = chan->getUsers();
			for (std::set<std::string>::const_iterator uit = users.begin(); uit != users.end(); ++uit) {
				if (*uit == nick) continue;
				Client* m = _clients.get_client(*uit);
				if (m) sndmsg(*m, quitMsg);
			}
			chan->removeUser(nick);
			if (chan->getUsers().empty())
				_channels.remove_channel(*it);
		}
	}

	_clients.remove_client(clientSocket);
	_receiveBuffers.erase(clientSocket);
	_sendBuffers.erase(clientSocket);
	_peerHostnames.erase(clientSocket);
	close(clientSocket);
	_pollFds.erase(_pollFds.begin() + pollIndex);

	std::cout << "Client disconnected from fd "
		<< clientSocket << std::endl;
}

// store outgoing data until poll() reports that the socket is ready to write
void	Server::queueMessage(int clientSocket, const std::string& message){
	if (message.empty())
		return;
	if (_sendBuffers.find(clientSocket) == _sendBuffers.end())
		_sendBuffers[clientSocket] = "";

	// The caller provides the complete IRC message, including "\r\n".
	_sendBuffers[clientSocket].append(message);
	setWriteMonitoring(clientSocket, true);
	flushSendBuffer(clientSocket);
}

// control whether poll() should report when a client socket can accept data
void	Server::setWriteMonitoring(int clientSocket, bool enabled){
	std::size_t	i;

	i = 0;
	while (i < _pollFds.size()){
		if (_pollFds[i].fd == clientSocket){
			if (enabled)
				_pollFds[i].events |= POLLOUT;
			else
				_pollFds[i].events &= ~POLLOUT;
			return;
		}
		i++;
	}
}

// send as much queued data as the non-blocking client socket currently accepts
bool	Server::sendBufferedData(std::size_t pollIndex){
	std::map<int, std::string>::iterator	bufferIt;
	int									clientSocket;
	ssize_t								bytesSent;

	clientSocket = _pollFds[pollIndex].fd;
	bufferIt = _sendBuffers.find(clientSocket);

	if (bufferIt == _sendBuffers.end())
		return (true);
	if (bufferIt->second.empty()){
		setWriteMonitoring(clientSocket, false);
		return (true);
	}

	bytesSent = send(clientSocket, bufferIt->second.c_str(),
			bufferIt->second.length(), 0);
	if (bytesSent == -1){
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return (true);
		std::cerr << "Failed to send data to client fd "
			<< clientSocket << std::endl;
		disconnectClient(pollIndex);
		return (false);
	}

	// send() may write only part of the message, so keep the remainder.
	bufferIt->second.erase(0, static_cast<std::size_t>(bytesSent));

	if (bufferIt->second.empty())
		setWriteMonitoring(clientSocket, false);

	return (true);
}

void	Server::flushSendBuffer(int clientSocket){
	std::map<int, std::string>::iterator	bufferIt = _sendBuffers.find(clientSocket);

	if (bufferIt == _sendBuffers.end() || bufferIt->second.empty())
		return;

	while (!bufferIt->second.empty()) {
		ssize_t	bytesSent = send(clientSocket, bufferIt->second.c_str(),
				bufferIt->second.length(), 0);
		if (bytesSent == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			if (errno == EPIPE) {
				disconnectClient(clientSocket);
				return;
			}
			break;
		}
		bufferIt->second.erase(0, static_cast<std::size_t>(bytesSent));
	}

	if (bufferIt->second.empty())
		setWriteMonitoring(clientSocket, false);
}

int	Server::sndmsg(const Client& clnt, const std::string& msg) {
	queueMessage(clnt.getFd(), msg);
	return (0);
}

void	Server::disconnectClient(int fd) {
	if (_clients.get_client_fd(fd) == NULL
		&& _receiveBuffers.find(fd) == _receiveBuffers.end()
		&& _sendBuffers.find(fd) == _sendBuffers.end()) {
		for (std::size_t i = 0; i < _pollFds.size(); i++) {
			if (_pollFds[i].fd == fd) {
				close(fd);
				_pollFds.erase(_pollFds.begin() + i);
				break;
			}
		}
		return;
	}
	_clients.remove_client(fd);
	_receiveBuffers.erase(fd);
	_sendBuffers.erase(fd);
	_peerHostnames.erase(fd);
	for (std::size_t i = 0; i < _pollFds.size(); i++) {
		if (_pollFds[i].fd == fd) {
			close(fd);
			_pollFds.erase(_pollFds.begin() + i);
			return;
		}
	}
}

// preserve received data between recv() calls until full command lines exist
void	Server::bufferClientInput(int clientSocket, const char* data,
									std::size_t dataLength){
	_receiveBuffers[clientSocket].append(data, dataLength);
	processClientBuffer(clientSocket);
}

// extract every newline-terminated command and keep any incomplete remainder
void	Server::processClientBuffer(int clientSocket){
	std::string&	clientBuffer = _receiveBuffers[clientSocket];
	std::string		command;
	std::size_t		lineEnd;

	lineEnd = clientBuffer.find('\n');
	while (lineEnd != std::string::npos){
		command = clientBuffer.substr(0, lineEnd);
		clientBuffer.erase(0, lineEnd + 1);

		// IRC normally uses "\r\n"; also tolerate commands ending only in "\n"
		if (command.empty() == false
			&& command[command.length() - 1] == '\r')
			command.erase(command.length() - 1);
		if (command.empty() == false)
			handleCompleteCommand(clientSocket, command);
		if (_receiveBuffers.find(clientSocket) == _receiveBuffers.end())
			return;

		lineEnd = clientBuffer.find('\n');
	}
}

void	Server::handleCompleteCommand(int clientSocket,
		const std::string& command){
	executecmd(clientSocket, command);
}

// entry point for the dispatch pipeline: build a Command from a raw line
// and let it route to the matching handler (see srcs/Command/Command.cpp)
int	Server::executecmd(int fd, const std::string& raw){
	try {
		Command	cmd(fd, raw);
		return cmd.execute(*this);
	} catch (const Command::MalformedException&) {
		return 1;
	}
}

Clients&	Server::getClients(){
	return _clients;
}

Channels&	Server::getChannels(){
	return _channels;
}

const std::string&	Server::getPassword() const {
	return _password;
}

const std::string&	Server::getServerName() const {
	return _serverName;
}

std::string	Server::getPeerHostname(int fd) const {
	std::map<int, std::string>::const_iterator it = _peerHostnames.find(fd);

	if (it != _peerHostnames.end())
		return it->second;
	return "127.0.0.1";
}
