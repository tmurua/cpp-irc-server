/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tmurua <tmurua@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/04 18:03:21 by tmurua            #+#    #+#             */
/*   Updated: 2026/06/04 19:25:10 by tmurua           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#include <cstdlib>		// std::strtol
#include <stdexcept>	// std::invalid_argument

Server::Server(const std::string& port, const std::string& password)
	: _port(parsePort(port)), _password(password) {
	if (_password.empty())
		throw std::invalid_argument("password cannot be empty");
}

Server::~Server() {}

int	Server::parsePort(const std::string& port) const {
	long	portNumber;

if (port.empty())
throw std::invalid_argument("port cannot be empty");
	if (isOnlyDigits(port) == false)
		throw std::invalid_argument("port must contain only digits");

	portNumber = std::strtol(port.c_str(), NULL, 10);
	// TCP ports use 16 bits numbers: 2^16 = 65536 possible values 
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
