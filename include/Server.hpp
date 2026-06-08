/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmurua <tmurua@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/04 17:43:29 by tmurua            #+#    #+#             */
/*   Updated: 2026/06/08 12:50:30 by tmurua           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <poll.h>	// pollfd, POLLIN
#include <vector>	// std::vector
#include <cstddef>	// std::size_t

class Server {
private:
	int					_port;
	std::string			_password;
	int					_serverSocket;
	std::vector<pollfd>	_pollFds;

	int		parsePort(const std::string& port) const;
	bool	isOnlyDigits(const std::string& text) const;
	void	createSocket();
	void	enableAddressReuse();
	void	bindSocketToPort();
	void	startListening();
	void	setSocketNonBlocking(int socketFd);
	void	addSocketToPoll(int socketFd);
	void	runEventLoop();
	void	acceptNewClient();
	bool	readFromClient(std::size_t pollIndex);
	void	disconnectClient(std::size_t pollIndex);

public:
	Server(const std::string& port, const std::string& password);
	~Server();

	void	start();
};

#endif
