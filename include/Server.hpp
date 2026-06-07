/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: johartma <johartma@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/04 17:43:29 by tmurua            #+#    #+#             */
/*   Updated: 2026/06/07 19:45:24 by johartma         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include "Client.hpp"
# include <poll.h>	// pollfd, POLLIN
# include <vector>	// std::vector

class Server {
private:
	int					_port;
	std::string			_password;
	int					_serverSocket;
	std::vector<pollfd>	_pollFds;
	std::vector<int>	_clientSockets;

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

public:
	Server(const std::string& port, const std::string& password);
	~Server();

	void	start();
	int		executecmd(const std::string& raw);
	int		sndmsg(const Client& clnt, const std::string& msg);
};

# endif
